#include <cSqlFileParser.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

SqlFileParser::~SqlFileParser() {
}

bool
SqlFileParser::loadQueriesFromFile(std::shared_ptr<std::vector<std::string>> queryList,
                                   std::string in_filename)
{
  // Caller must pass a valid container
  if (!queryList) {
    std::cerr << "=> " << __PRETTY_FUNCTION__ << ": queryList is null" << std::endl;
    return false;
  }

  std::ifstream file(in_filename);
  if (!file.is_open()) {
    std::cerr << "=> Unable to open SQL file: " << in_filename << std::endl;
    std::cerr << std::strerror(errno) << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    queryList->push_back(line);
  }

  return true;
}
