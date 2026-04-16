#include <cSqlFileParser.hpp>
#include <fstream>
#include <iostream>
#include <cstring>

namespace
  {

  std::string
  trim(const std::string& s) {
    std::size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
      return "";
      }

    std::size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
    }

  bool
  isCommentLine(const std::string& s) {
    return s.rfind("#", 0) == 0 ||
      s.rfind("--", 0) == 0 ||
      s.rfind("//", 0) == 0;
    }
  }


SqlFileParser::~SqlFileParser() {
  }


bool
SqlFileParser::loadQueriesFromFile(std::shared_ptr<std::vector<std::string>> queryList,
const std::string& infileName) {
  if (!queryList) {
    std::cerr << "=> " << __PRETTY_FUNCTION__ << ": queryList is null" << std::endl;
    return false;
    }

  std::ifstream file(infileName);
  if (!file.is_open()) {
    std::cerr << "=> Unable to open SQL file: " << infileName << std::endl;
    std::cerr << std::strerror(errno) << std::endl;
    return false;
    }

  std::string line;
  while (std::getline(file, line)) {
    std::string trimmed = trim(line);

    if (trimmed.empty()) {
      continue;
      }

    if (isCommentLine(trimmed)) {
      continue;
      }

    queryList->push_back(trimmed);
    }

  return true;
  }
