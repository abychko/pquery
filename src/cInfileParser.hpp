#ifndef C_INFILEPARSER_HPP
#define C_INFILEPARSER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class InfileParser {
 public:
  virtual ~InfileParser() = default;

  virtual bool loadQueriesFromFile(
      std::shared_ptr<std::vector<std::string>> queryList,
      const std::string &infileName) = 0;

  virtual std::uint64_t getInfileSize(const std::string &infileName) const;
};
#endif
