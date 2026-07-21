#include <cstdint>
#include <eTypes.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef INIREADER_HPP
#define INIREADER_HPP

typedef std::unordered_map<std::string,
                           std::unordered_map<std::string, std::string>>
    configuration;

class INIReader {
 public:
  explicit INIReader(const std::string &filename);
  int ParseError() const { return _error; };
  std::vector<std::string> GetSections() const { return _sections; };
  std::string Get(const std::string &section, const std::string &name,
                  const std::string &default_value);
  int64_t GetInteger(const std::string &section, const std::string &name,
                     int64_t default_value);
  bool GetBoolean(const std::string &section, const std::string &name,
                  bool default_value);
  eDBTYPE getDbType(const std::string &section, const std::string &name,
                    eDBTYPE default_value);
  eINFILETYPE getInfileType(const std::string &section, const std::string &name,
                            eINFILETYPE default_value);

 private:
  int _error;
  std::vector<std::string> _sections;
  configuration map;
};
#endif
