#include <sys/types.h>
#include <cDbWorker.hpp>
#include <cIniReader.hpp>
#include <cLogger.hpp>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
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

  bool setupWorkerParams(struct workerParams &, std::string);
  eRETCODE createWorkerProcess(struct workerParams &);
  void reportConfigError(const std::string &secName, const std::string &msg);
  bool checkIntRange(const std::string &secName, const std::string &name,
                     std::int64_t value, std::int64_t min, std::int64_t max);

  struct workerProc {
    pid_t pid;
    std::string name;
    bool has_timeout;
    std::chrono::steady_clock::time_point deadline;
    bool term_sent;
    std::chrono::steady_clock::time_point kill_deadline;
  };
  std::vector<workerProc> activeWorkers;
  bool waitForWorkers();
  //
  std::shared_ptr<INIReader> configReader;
  std::shared_ptr<Logger> pqLogger;
  std::shared_ptr<DbWorker> dbWorker;
};
#endif
