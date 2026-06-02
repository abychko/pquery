#include <cDbWorker.hpp>
#include <cIniReader.hpp>
#include <cLogger.hpp>
#include <memory>
#include <string>
#include "eTypes.hpp"

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
  bool runWorkers();
  static void showHelp();
  void showVersion();
  void setConfigFilePath(std::string configPath) {
    configFilePath = configPath;
  }
  void setLogFilePath(std::string logPath) { logFilePath = logPath; }
  void logVersionInfo();

 private:
  std::string configFilePath;
  std::string logFilePath;
  void doCleanup(std::string);
  void logWorkerDetails(struct workerParams &);

#ifdef HAVE_MYSQL
  std::string getMySqlClientInfo();
#endif
#ifdef HAVE_PGSQL
  std::string getPgSqlClientInfo();
#endif

  eRETCODE createWorkerWithParams(std::string);
  void setupWorkerParams(struct workerParams &, std::string);
  eRETCODE createWorkerProcess(struct workerParams &);
  //
  std::shared_ptr<INIReader> configReader;
  std::shared_ptr<Logger> pqLogger;
  std::shared_ptr<DbWorker> dbWorker;
};
#endif
