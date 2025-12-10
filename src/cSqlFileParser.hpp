#ifndef SQLFILEPARSER_HPP
#define SQLFILEPARSER_HPP

#include <cInfileParser.hpp>
#include <vector>

class SqlFileParser : public InfileParser
  {

  std::vector<std::string> sqlLines;
  };
#endif
