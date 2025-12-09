#include <string>
#include <algorithm>
#include <cIniReader.hpp>
#include <cLogger.hpp>
#include <cDbWorker.hpp>

#ifndef PQUERY_HPP
#define PQUERY_HPP

class PQuery
  {

  public:
    PQuery();
    ~PQuery();
    bool prepareToRun();
    int run();
    bool initConfig();
    bool initLogger();
    bool parseCliOptions(int argc, char* argv[]);
    bool runWorkers();
    void showHelp();
    void showVersion();
    void setConfigFilePath(std::string configPath) { configFilePath = configPath; }
    void setLogFilePath(std::string logPath) { logFilePath = logPath; }
    void logVersionInfo();

  private:
    std::string configFilePath;
    std::string logFilePath;

    inline std::string
    toLowerCase(std::string str) {
      auto lowercased = str;
      std::transform (lowercased.begin(), lowercased.end(), lowercased.begin(), ::tolower);
      return lowercased;
      }
    inline std::string
    dbtype_str(eDBTYPE type) {
      switch (type) {
        case eMYSQL:
          return "MySQL";
        case ePGSQL:
          return "PostgreSQL";
        default:
          return "UNKNOWN";
        }
      }
    inline std::string
    infiletype_str(eINFILETYPE infiletype) {
      switch (infiletype) {
        case eSQL:
          return "SQL";
        case eGENLOG:
          return "General Log";
        case eBINLOG:
          return "Binary Log";
        default:
          return "UNKNOWN TYPE";
        }
      }
    void  doCleanup(std::string);
    void logWorkerDetails(struct workerParams&);

#ifdef HAVE_MYSQL
    std::string getMySqlClientInfo();
#endif
#ifdef HAVE_PGSQL
    std::string getPgSqlClientInfo();
#endif

    wRETCODE createWorkerWithParams(std::string);
    void setupWorkerParams(struct workerParams&, std::string);
    wRETCODE createWorkerProcess(struct workerParams&);
//
    std::shared_ptr<INIReader> configReader;
    std::shared_ptr<Logger> pqLogger;
    std::shared_ptr<DbWorker> dbWorker;

  };
#endif
