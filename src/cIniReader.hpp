#ifndef _INIREADER_HPP_
#define _INIREADER_HPP_

#include <eTypes.hpp>
#include <string>
#include <unordered_map>
#include <vector>

typedef std::unordered_map<std::string,
                           std::unordered_map<std::string, std::string>>
    configuration;

class INIReader {
 public:
  explicit INIReader(std::string filename);
  int ParseError() const { return _error; };
  std::vector<std::string> GetSections() const { return _sections; };
  std::string Get(std::string, std::string name, std::string default_value);
  int GetInteger(std::string section, std::string name, int default_value);
  bool GetBoolean(std::string section, std::string name, bool default_value);
  eDBTYPE getDbType(std::string section, std::string name,
                    eDBTYPE default_value);
  eINFILETYPE getInfileType(std::string section, std::string name,
                            eINFILETYPE default_value);

 private:
  int _error;
  std::vector<std::string> _sections;
  configuration map;
};
#endif
