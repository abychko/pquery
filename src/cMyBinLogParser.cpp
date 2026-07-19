#include <cMyBinLogParser.hpp>
#include <iostream>

MyBinLogParser::MyBinLogParser() = default;

MyBinLogParser::~MyBinLogParser() = default;

bool MyBinLogParser::loadQueriesFromFile(
    std::shared_ptr<std::vector<std::string>>, const std::string &) {
  std::cerr << "=> BinLog parsing not implemented yet" << std::endl;
  return false;
}
