#ifndef _CMYBINLOGPARSER_HPP_
#define _CMYBINLOGPARSER_HPP_

#include <cInfileParser.hpp>

class MyBinLogParser : public InfileParser
  {
  public:
    MyBinLogParser();
    ~MyBinLogParser() override;

    bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>>,
      const std::string&) override;
  };
#endif
