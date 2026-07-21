#include <algorithm>
#include <cIniReader.hpp>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {
bool isSpaceChar(char c) {
  return std::isspace(static_cast<unsigned char>(c)) != 0;
}

std::size_t skipWs(const std::string &line, std::size_t pos) {
  while (pos < line.size() && isSpaceChar(line[pos])) {
    ++pos;
  }
  return pos;
}

bool isBlank(const std::string &line) { return skipWs(line, 0) == line.size(); }

bool isComment(const std::string &line) {
  std::size_t i = skipWs(line, 0);
  if (i >= line.size()) {
    return false;
  }
  if (line[i] == ';' || line[i] == '#') {
    return true;
  }
  return (i + 1 < line.size() && line[i] == '-' && line[i + 1] == '-');
}

bool tryParseSection(const std::string &line, std::string &sectionName) {
  std::size_t i = skipWs(line, 0);
  if (i >= line.size() || line[i] != '[') {
    return false;
  }
  std::size_t close = line.find(']', i + 1);
  if (close == std::string::npos || close == i + 1 ||
      close + 1 != line.size()) {
    return false;
  }
  sectionName = line.substr(i + 1, close - i - 1);
  return true;
}

bool tryParseKeyValue(const std::string &line, std::string &key,
                      std::string &value) {
  std::size_t n = line.size();
  std::size_t i = skipWs(line, 0);
  if (i == n) {
    return false;
  }
  std::size_t kb = i;
  ++i;  // first key char: any non-whitespace, including '='
  while (i < n && line[i] != ' ' && line[i] != '\t' && line[i] != '=') {
    ++i;
  }
  std::string keyCandidate = line.substr(kb, i - kb);

  i = skipWs(line, i);
  if (i == n || line[i] != '=') {
    return false;
  }
  i = skipWs(line, i + 1);
  if (i == n) {
    return false;  // empty value
  }

  std::size_t vb = i;
  while (i < n && !isSpaceChar(line[i])) {
    ++i;
  }
  std::size_t ve = i;

  while (i < n) {
    std::size_t ws = i;
    while (i < n && isSpaceChar(line[i])) {
      ++i;
    }
    if (i == n) {
      break;  // trailing whitespace, stop
    }
    if (i - ws != 1) {
      return false;  // 2+ whitespace between words: malformed
    }
    while (i < n && !isSpaceChar(line[i])) {
      ++i;
    }
    ve = i;
  }

  key = keyCandidate;
  value = line.substr(vb, ve - vb);
  return true;
}
}  // namespace

INIReader::INIReader(std::string filename) {
  std::string current_section;
  std::ifstream cfg;

  _error = 0;

  cfg.open(filename);

  if (!cfg) {
    _error = -1;
    return;
  }

  int line_no = 0;
  for (std::string line; std::getline(cfg, line);) {
    ++line_no;
    if (line.empty()) {
      continue;
    }
    if (isBlank(line)) {
      continue;
    }
    if (isComment(line)) {
      continue;
    }
    {
      std::string sectionName;
      if (tryParseSection(line, sectionName)) {
        current_section = sectionName;
        _sections.push_back(current_section);
        continue;
      }
    }
    {
      std::string key, value;
      if (tryParseKeyValue(line, key, value)) {
        map[current_section][key] = value;
        continue;
      }
    }
    if (_error == 0) {
      _error = line_no;  // first malformed line, keep parsing to collect all
                         // sections
    }
  }  // for()
  cfg.close();
}

eINFILETYPE INIReader::getInfileType(std::string section, std::string name,
                                     eINFILETYPE default_value) {
  std::string valstr = Get(section, name, "");
  if (valstr.empty()) {
    return default_value;
  }
  std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
  if (valstr == "sql") {
    return eSQL;
  }
  if (valstr == "genlog") {
    return eGENLOG;
  }
  if (valstr == "binlog") {
    return eBINLOG;
  }
  return eUNKNOWN;
}

eDBTYPE INIReader::getDbType(std::string section, std::string name,
                             eDBTYPE default_value) {
  std::string valstr = Get(section, name, "");
  if (valstr.empty()) {
    return default_value;
  }
  std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
  if ((valstr == "mysql") || (valstr == "mariadb")) {
    return eMYSQL;
  }
  if ((valstr == "pgsql") || (valstr == "postgres") ||
      (valstr == "postgresql")) {
    return ePGSQL;
  }
  // throw std::invalid_argument("Invalid value for DB TYPE: " + valstr);
  return eNONE;
}

std::string INIReader::Get(std::string section, std::string name,
                           std::string default_value) {
  auto section_it = map.find(section);
  if (section_it == map.end()) {
    return default_value;
  }

  auto value_it = section_it->second.find(name);
  if (value_it == section_it->second.end()) {
    return default_value;
  }

  return (!value_it->second.empty()) ? value_it->second : default_value;
}

bool INIReader::GetBoolean(std::string section, std::string name,
                           bool default_value) {
  std::string valstr = Get(section, name, "");
  // Convert to lower case to make string comparisons case-insensitive
  std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
  if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1") {
    return true;
  }
  if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0") {
    return false;
  }
  return default_value;
}

int64_t INIReader::GetInteger(std::string section, std::string name,
                              int64_t default_value) {
  std::string valstr = Get(section, name, "");
  if (valstr.empty()) {
    return default_value;
  }

  std::istringstream vss;
  vss.str(valstr);
  int64_t ipart = 0;
  char cpart = 0;  // can be K/M/G/T

  vss >> ipart;
  if (vss.fail()) {
    throw std::invalid_argument("Invalid value for " + name + ": " + valstr);
  }

  vss >> cpart;

  if (cpart == 0) {
    return ipart;
  }

  int64_t multiplier = 1;
  switch (cpart) {
    case 'k':
    case 'K':
      multiplier = 1024LL;
      break;
    case 'm':
    case 'M':
      multiplier = 1024LL * 1024;
      break;
    case 'g':
    case 'G':
      multiplier = 1024LL * 1024 * 1024;
      break;
    case 't':
    case 'T':
      multiplier = 1024LL * 1024 * 1024 * 1024;
      break;
    default:
      throw std::invalid_argument("Invalid value for " + name + ": " + valstr);
      break;
  }

  // guard against overflow instead of wrapping around silently (UB for
  // signed types)
  if ((ipart != 0) &&
      ((ipart > std::numeric_limits<int64_t>::max() / multiplier) ||
       (ipart < std::numeric_limits<int64_t>::min() / multiplier))) {
    throw std::overflow_error("Value out of range for " + name + ": " + valstr);
  }

  return (ipart * multiplier);
}
