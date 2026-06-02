#include <cMyBinLogParser.hpp>

MyBinLogParser::MyBinLogParser() = default;

MyBinLogParser::~MyBinLogParser() = default;

bool MyBinLogParser::loadQueriesFromFile(
    std::shared_ptr<std::vector<std::string>>, const std::string &) {
  return true;
}
