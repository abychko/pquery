#ifndef MYBINLOGPARSER_HPP
#define MYBINLOGPARSER_HPP

#include <cInfileParser.hpp>

class MyBinLogParser : public InfileParser
  {
  public:
    MyBinLogParser();
    ~MyBinLogParser();
    bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>>, std::string);
  };
#endif
