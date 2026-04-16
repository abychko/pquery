#ifndef _CSQLFILEPARSER_HPP_
#define _CSQLFILEPARSER_HPP_

#include <cInfileParser.hpp>

class SqlFileParser : public InfileParser
{
public:
  ~SqlFileParser() override;

  bool loadQueriesFromFile(
    std::shared_ptr<std::vector<std::string>> queryList,
    const std::string& infileName) override;
};

#endif
