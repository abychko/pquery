#include <cMyGenLogParser.hpp>
#include <iostream>

MyGenLogParser::MyGenLogParser() = default;

MyGenLogParser::~MyGenLogParser() = default;

bool MyGenLogParser::loadQueriesFromFile(
    std::shared_ptr<std::vector<std::string>>, const std::string &) {
  std::cerr << "=> GenLog parsing not implemented yet" << std::endl;
  return false;
}
