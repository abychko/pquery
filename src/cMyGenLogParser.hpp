#ifndef MYGENLOGPARSER_HPP
#define MYGENLOGPARSER_HPP

#include <cInfileParser.hpp>

class MyGenLogParser : public InfileParser
  {
  public:
    MyGenLogParser();
    ~MyGenLogParser();
    bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>>, std::string);

  };
#endif
