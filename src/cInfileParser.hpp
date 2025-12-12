#ifndef INFILEPARSER_HPP
#define INFILEPARSER_HPP

#include <fstream>
#include <string>
#include <memory>
#include <cstdint>
#include <vector>

class InfileParser
  {

  public:
    InfileParser();
    virtual ~InfileParser();
    std::uint64_t getInfileSize(std::string);
    virtual bool loadQueriesFromFile(std::shared_ptr<std::vector<std::string>>, std::string) = 0;
  private:
// struct workerParams mParams;

  };
#endif
