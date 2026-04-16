#ifndef _INFILEPARSER_HPP_
#define _INFILEPARSER_HPP_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class InfileParser
{
public:
  InfileParser();
  virtual ~InfileParser();

  std::uint64_t getInfileSize(const std::string&) const;
  virtual bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>>,
                                   const std::string&) = 0;
};

#endif
