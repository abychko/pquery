#ifndef SQLFILEPARSER_HPP
#define SQLFILEPARSER_HPP

#include <cInfileParser.hpp>
#include <vector>

class SqlFileParser : public InfileParser
  {
  public:
    ~SqlFileParser();
    bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>>, std::string);
  private:
  };
#endif
