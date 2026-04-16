#include <cInfileParser.hpp>
#include <cstdint>
#include <iostream>
#include <fstream>

InfileParser::InfileParser() {

  }


InfileParser::~InfileParser() {

  }


std::uint64_t
InfileParser::getInfileSize(const std::string& infile_name) const
  {
  std::ifstream file(infile_name, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    return 0;
    }
  return static_cast<std::uint64_t>(file.tellg());
  }
