#ifndef MYSQLPARSER_HPP
#define MYSQLPARSER_HPP

#include <cInfileParser.hpp>
#include <vector>

class MySQLParser : public InfileParser
  {

  std::vector<std::string> sqlLines;
  };
#endif
