#include <algorithm>
#include <cIniReader.hpp>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>
#include <stdexcept>

INIReader::INIReader(std::string filename) {
  static const std::regex blank_regex("^\\s*$");
  static const std::regex comment_regex("^\\s*(?:;|#|--).*$");
  static const std::regex section_regex{R"x(\s*\[([^\]]+)\])x"};
  static const std::regex value_regex{
      R"x(\s*(\S[^ \t=]*)\s*=\s*((\s?\S+)+)\s*$)x"};
  std::smatch pieces;
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
    if (std::regex_match(line, pieces, blank_regex)) {
      continue;
    }
    if (std::regex_match(line, pieces, comment_regex)) {
      continue;
    }
    if (std::regex_match(line, pieces, section_regex)) {
      if (pieces.size() == 2) {  // exactly one match
        current_section = pieces[1].str();
        _sections.push_back(current_section);
      }
      continue;
    }
    if (std::regex_match(line, pieces, value_regex)) {
      if (pieces.size() == 4) {
        map[current_section][pieces[1].str()] = pieces[2].str();
      }
      continue;
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
