#include <cPQuery.hpp>
#include <cWorkerConfig.hpp>
#include <cWorkerPool.hpp>
#include <cstdlib>
#include <hCommon.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "eTypes.hpp"

PQuery::PQuery() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
}

PQuery::~PQuery() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
}

bool PQuery::initLogger() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  pqLogger = std::make_shared<Logger>();

  if (!options.logFilePath.empty()) {
    pqLogger->setLogFilePath(options.logFilePath);
  }

  std::string masterLogFile;
  std::string master_logdir = configReader->Get("master", "logdir", "/tmp");
  std::string master_logfile =
      configReader->Get("master", "logfile",
                        std::string("pquery") + PQMAJVERSION + "-master.log");
  masterLogFile = master_logdir + FSSEP + master_logfile;
  return pqLogger->initLogFile(masterLogFile);
}

bool PQuery::initConfig() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  configReader = std::make_shared<INIReader>(options.configFilePath);
  int parseerr = configReader->ParseError();

  if (parseerr < 0) {
    std::cerr << "Can't load config from file \"" + options.configFilePath +
                     "\""
              << '\n';
    return false;
  }
  if (parseerr > 0) {
    std::cerr << "Config parse error!" << '\n';
    std::cerr << "File: " << options.configFilePath << '\n';
    std::cerr << "Line: " << parseerr << '\n';
    return false;
  }
  return true;
}

bool PQuery::prepareToRun() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  if (!initConfig()) {
    return false;
  }
  if (!initLogger()) {
    return false;
  }
  logVersionInfo(*pqLogger);
  return true;
}

int PQuery::run() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  showVersion();
  if (!prepareToRun()) {
    return EXIT_FAILURE;
  }

  std::string masterLogDir = configReader->Get("master", "logdir", "/tmp");
  std::vector<workerParams> validParams;
  WorkerConfig config(configReader, pqLogger);
  if (!config.buildAllWorkerParams(validParams)) {
    return EXIT_FAILURE;
  }

  WorkerPool pool(pqLogger, masterLogDir);
  return pool.runAll(validParams) ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool PQuery::parseCliOptions(int argc, char *argv[]) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  return ::parseCliOptions(argc, argv, options);
}
