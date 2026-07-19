#include <cSqlFileParser.hpp>

#include <cstring>
#include <fstream>
#include <iostream>

namespace {
enum class ScanState {
  Normal,
  SingleQuote,
  DoubleQuote,
  Backtick,
  LineComment,
  BlockComment
};

std::string trim(const std::string &s) {
  std::size_t first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return "";
  }

  std::size_t last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, last - first + 1);
}

bool openSqlFile(std::ifstream &file, const std::string &infileName) {
  file.open(infileName);
  if (!file.is_open()) {
    std::cerr << "=> Unable to open SQL file: " << infileName << std::endl;
    std::cerr << std::strerror(errno) << std::endl;
    return false;
  }

  return true;
}

void pushStatement(std::shared_ptr<std::vector<std::string>> queryList,
                   std::string &statement) {
  std::string trimmed = trim(statement);
  if (!trimmed.empty()) {
    queryList->push_back(trimmed);
  }

  statement.clear();
}

bool startsLineComment(const std::string &text, std::size_t pos) {
  if (text[pos] == '#') {
    return true;
  }

  if (text[pos] == '/' && pos + 1 < text.size() && text[pos + 1] == '/') {
    return true;
  }

  if (text[pos] == '-' && pos + 2 < text.size() && text[pos + 1] == '-') {
    char next = text[pos + 2];
    return next == ' ' || next == '\t' || next == '\r' || next == '\n';
  }

  return false;
}

bool startsBlockComment(const std::string &text, std::size_t pos) {
  return text[pos] == '/' && pos + 1 < text.size() && text[pos + 1] == '*';
}

bool isDelimiterCommand(const std::string &rawLine, std::string &newDelimiter) {
  std::string line = trim(rawLine);

  const std::string prefixUpper = "DELIMITER ";
  const std::string prefixLower = "delimiter ";

  if (line.size() <= prefixUpper.size()) {
    return false;
  }

  bool hasPrefix =
      line.rfind(prefixUpper, 0) == 0 || line.rfind(prefixLower, 0) == 0;

  if (!hasPrefix) {
    return false;
  }

  std::string value = line.substr(prefixUpper.size());
  value = trim(value);

  if (value.empty()) {
    return false;
  }

  newDelimiter = value;
  return true;
}

bool startsWithDelimiter(const std::string &text, std::size_t pos,
                         const std::string &delimiter) {
  if (delimiter.empty()) {
    return false;
  }

  if (pos + delimiter.size() > text.size()) {
    return false;
  }

  return text.compare(pos, delimiter.size(), delimiter) == 0;
}

void consumeNormalChar(const std::string &text, std::size_t &pos,
                       ScanState &state, std::string &statement,
                       std::shared_ptr<std::vector<std::string>> queryList,
                       const std::string &delimiter) {
  char c = text[pos];

  if (startsWithDelimiter(text, pos, delimiter)) {
    pushStatement(queryList, statement);
    pos += delimiter.size() - 1;
    return;
  }

  if (startsLineComment(text, pos)) {
    state = ScanState::LineComment;
    if (c == '/' || c == '-') {
      ++pos;
    }
    return;
  }

  if (startsBlockComment(text, pos)) {
    state = ScanState::BlockComment;
    ++pos;
    return;
  }

  if (c == '\'') {
    state = ScanState::SingleQuote;
    statement.push_back(c);
    return;
  }

  if (c == '"') {
    state = ScanState::DoubleQuote;
    statement.push_back(c);
    return;
  }

  if (c == '`') {
    state = ScanState::Backtick;
    statement.push_back(c);
    return;
  }

  statement.push_back(c);
}

void consumeQuotedChar(const std::string &text, std::size_t &pos,
                       ScanState &state, std::string &statement, char quote) {
  char c = text[pos];
  statement.push_back(c);

  if (c == '\\' && pos + 1 < text.size()) {
    ++pos;
    statement.push_back(text[pos]);
    return;
  }

  if (c == quote) {
    state = ScanState::Normal;
  }
}

void consumeBacktickChar(const std::string &text, std::size_t &pos,
                         ScanState &state, std::string &statement) {
  char c = text[pos];
  statement.push_back(c);

  if (c == '`') {
    state = ScanState::Normal;
  }
}

void consumeLineCommentChar(const std::string &text, std::size_t pos,
                            ScanState &state) {
  if (text[pos] == '\n') {
    state = ScanState::Normal;
  }
}

void consumeBlockCommentChar(const std::string &text, std::size_t &pos,
                             ScanState &state) {
  if (text[pos] == '*' && pos + 1 < text.size() && text[pos + 1] == '/') {
    state = ScanState::Normal;
    ++pos;
  }
}

bool splitStatements(std::ifstream &file,
                     std::shared_ptr<std::vector<std::string>> queryList) {
  ScanState state = ScanState::Normal;
  std::string statement;
  std::string line;
  std::string delimiter = ";";

  while (std::getline(file, line)) {
    std::string newDelimiter;
    if (state == ScanState::Normal && isDelimiterCommand(line, newDelimiter)) {
      delimiter = newDelimiter;
      continue;
    }

    line.push_back('\n');

    for (std::size_t pos = 0; pos < line.size(); ++pos) {
      switch (state) {
        case ScanState::Normal:
          consumeNormalChar(line, pos, state, statement, queryList, delimiter);
          break;
        case ScanState::SingleQuote:
          consumeQuotedChar(line, pos, state, statement, '\'');
          break;
        case ScanState::DoubleQuote:
          consumeQuotedChar(line, pos, state, statement, '"');
          break;
        case ScanState::Backtick:
          consumeBacktickChar(line, pos, state, statement);
          break;
        case ScanState::LineComment:
          consumeLineCommentChar(line, pos, state);
          break;
        case ScanState::BlockComment:
          consumeBlockCommentChar(line, pos, state);
          break;
      }
    }
  }

  pushStatement(queryList, statement);
  return true;
}
}  // namespace

bool SqlFileParser::loadQueriesFromFile(
    std::shared_ptr<std::vector<std::string>> queryList,
    const std::string &infileName) {
  if (!queryList) {
    std::cerr << "=> " << __PRETTY_FUNCTION__ << ": queryList is null"
              << std::endl;
    return false;
  }

  std::ifstream file;
  if (!openSqlFile(file, infileName)) {
    return false;
  }

  return splitStatements(file, queryList);
}
