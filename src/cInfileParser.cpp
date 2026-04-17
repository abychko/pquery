#include <cInfileParser.hpp>
#include <fstream>

std::uint64_t
InfileParser::getInfileSize(const std::string& infileName) const
  {
  std::ifstream file(infileName, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    return 0;
    }

  return static_cast<std::uint64_t>(file.tellg());
  }
