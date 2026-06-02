#include <cMyGenLogParser.hpp>

MyGenLogParser::MyGenLogParser() = default;

MyGenLogParser::~MyGenLogParser() = default;

bool MyGenLogParser::loadQueriesFromFile(
    std::shared_ptr<std::vector<std::string>>, const std::string &) {
  return true;
}
