#include <cInfileParser.hpp>
#include <memory>
#include <string>
#include <vector>

#ifndef CMYGENLOGPARSER_HPP
#define CMYGENLOGPARSER_HPP

class MyGenLogParser : public InfileParser {
 public:
  MyGenLogParser();
  ~MyGenLogParser() override;

  bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>>,
                           const std::string &) override;
};
#endif
