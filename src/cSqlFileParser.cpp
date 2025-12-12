#include <cSqlFileParser.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

SqlFileParser::~SqlFileParser() {

}

bool
SqlFileParser::loadQueriesFromFile(std::shared_ptr<std::vector<std::string>> queryList, std::string in_filename) {
  std::ifstream file;
  file.open(in_filename);
  if(!file.is_open()) {
    std::cerr << "=> Unable to open SQL file: " << in_filename << std::endl;
    std::cerr << std::strerror(errno) << std::endl;
    return false;
    }

  std::string _str;
  while (std::getline(file, _str)) {
    queryList->push_back(_str);
    }

  file.close();
  return true;
  }
