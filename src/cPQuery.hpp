#include <cCli.hpp>
#include <cIniReader.hpp>
#include <cLogger.hpp>
#include <memory>
#include <string>

#ifndef PQUERY_HPP
#define PQUERY_HPP

class PQuery {
 public:
  PQuery();
  ~PQuery();
  bool prepareToRun();
  int run();
  bool initConfig();
  bool initLogger();
  bool parseCliOptions(int argc, char *argv[]);

 private:
  CliOptions options;
  //
  std::shared_ptr<INIReader> configReader;
  std::shared_ptr<Logger> pqLogger;
};
#endif
