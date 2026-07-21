#include <memory>
#include <string>

#ifndef CCLI_HPP
#define CCLI_HPP

class Logger;

struct CliOptions {
  std::string configFilePath{"pquery.cfg"};
  std::string logFilePath{};
};

bool parseCliOptions(int argc, char *argv[], CliOptions &opts);
void showHelp();
void showVersion();
void logVersionInfo(Logger &logger);

#endif
