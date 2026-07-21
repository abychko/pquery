#ifndef C_SQLFILEPARSER_HPP
#define C_SQLFILEPARSER_HPP

#include <cInfileParser.hpp>
#include <memory>
#include <string>
#include <vector>

class SqlFileParser : public InfileParser {
 public:
  explicit SqlFileParser(bool dollarQuoting = false);
  ~SqlFileParser() override = default;

  bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>> queryList,
                           const std::string &infileName) override;

 private:
  bool mDollarQuoting;
};

#endif
