#include <cInfileParser.hpp>
#include <cstdint>
#include <iostream>


InfileParser::InfileParser() {

}

InfileParser::~InfileParser() {

}

std::uint64_t
InfileParser::getInfileSize(std::string infile_name) {
  std::ifstream file(infile_name, std::ios::binary | std::ios::ate);
  if (!file) {
    std::cerr << "=> Unable to open file " << infile_name << std::endl;
    return 1;
    }
// fileSize in bytes
  std::uint64_t fileSize = file.tellg();
  if (file.is_open()) { file.close(); }
  return fileSize;
  }
