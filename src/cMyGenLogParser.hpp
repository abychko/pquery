#ifndef _CMYGENLOGPARSER_HPP_
#define _CMYGENLOGPARSER_HPP_

#include <cInfileParser.hpp>

class MyGenLogParser : public InfileParser
  {
  public:
    MyGenLogParser();
    ~MyGenLogParser() override;

    bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>>,
      const std::string&) override;
  };
#endif
