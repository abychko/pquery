#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <regex>
#include <cIniReader.hpp>

INIReader::INIReader(std::string filename) {
  static const std::regex comment_regex("^\\s*[;#].*$");
  static const std::regex section_regex{R"x(\s*\[([^\]]+)\])x"};
  static const std::regex value_regex{R"x(\s*(\S[^ \t=]*)\s*=\s*((\s?\S+)+)\s*$)x"};
  std::smatch pieces;
  std::string current_section;
  std::ifstream cfg;

  cfg.open(filename);

  if(!cfg) { _error = -1; }

  for (std::string line; std::getline(cfg, line);) {
    if (line.empty()) { continue; }
    if (std::regex_match(line, pieces, section_regex)) {
      if (pieces.size() == 2) {                   // exactly one match
        current_section = pieces[1].str();
        _sections.push_back(current_section);
        }
      }
    if (std::regex_match(line, pieces, value_regex)) {
      if (pieces.size() == 4) {
        map[current_section][pieces[1].str()] = pieces[2].str();
        }
      }
    }                                             // for()
  cfg.close();
  _error = 0;
  }

eDBTYPE
INIReader::getDbType(std::string section, std::string name, eDBTYPE default_value) {
  std::string valstr = Get(section, name, "");
  std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
  if(valstr.empty()) { return default_value; }
  if((valstr == "mysql") || (valstr == "mariadb")) { return eMYSQL; }
  if((valstr == "pgsql") || (valstr == "postgres") || (valstr == "postgresql"))  { return ePGSQL; }
  if((valstr == "mongo") || (valstr == "mongodb")) { return eMONGO; }
//throw std::logic_error("Invalid value for DB TYPE: " + valstr);
  return eNONE;
  }

std::string
INIReader::Get(std::string section, std::string name, std::string default_value) {
  std::string value = map[section][name];
  return (!value.empty()) ? value : default_value;
  }

bool
INIReader::GetBoolean(std::string section, std::string name, bool default_value) {
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


int
INIReader::GetInteger(std::string section, std::string name, int default_value) {
  std::string valstr = Get(section, name, "");
  if(valstr.empty()) { return default_value; }

  std::istringstream vss;
  vss.str(valstr);
  int ipart = 0;
  char cpart = 0;                                 //can be K/M/G

  vss >> ipart;
  if(vss.fail()) {
    throw std::invalid_argument("Invalid value for " + name + ": " + valstr);
    }

  vss >> cpart;

  if (cpart == 0) {
    return ipart;
    }

  switch (cpart) {
    case 'k':
    case 'K': return (ipart * 1024);
    break;
    case 'm':
    case 'M': return (ipart * 1024 * 1024);
    break;
    case 'g':
    case 'G': return (ipart * 1024 * 1024 * 1024);
    break;
    default:
      throw std::invalid_argument("Invalid value for " + name + ": " + valstr);
      break;
    }
  return default_value;
  }
