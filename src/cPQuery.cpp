#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cPQuery.hpp>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <hCommon.hpp>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "eTypes.hpp"
//
#ifdef HAVE_MYSQL
#include <mysql.h>
#include <cMysqlWorker.hpp>
#endif

#ifdef HAVE_PGSQL
#include <pg_config.h>
#include <cPgsqlWorker.hpp>
#endif

PQuery::PQuery() : configFilePath("pquery.cfg") {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
}

PQuery::~PQuery() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
}

#ifdef HAVE_MYSQL
std::string PQuery::getMySqlClientInfo() { return mysql_get_client_info(); }
#endif

#ifdef HAVE_PGSQL
std::string PQuery::getPgSqlClientInfo() { return std::string(PG_VERSION); }
#endif

bool PQuery::initLogger() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  pqLogger = std::make_shared<Logger>();

  if (!logFilePath.empty()) {
    pqLogger->setLogFilePath(logFilePath);
  }

  std::string masterLogFile;
  std::string master_logdir = configReader->Get("master", "logdir", "/tmp");
  std::string master_logfile =
      configReader->Get("master", "logfile",
                        std::string("pquery") + PQMAJVERSION + "-master.log");
  masterLogFile = master_logdir + FSSEP + master_logfile;
  return pqLogger->initLogFile(masterLogFile);
}

void PQuery::logVersionInfo() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  auto now = std::chrono::system_clock::now();
  std::time_t start_time = std::chrono::system_clock::to_time_t(now);
  pqLogger->addRecordToLog("* PQuery version: " + std::string(PQVERSION));
  pqLogger->addRecordToLog("* PQuery revision: " + std::string(PQREVISION));
  pqLogger->addRecordToLog("* PQuery revision date: " + std::string(PQRELDATE));
  pqLogger->addRecordToLog("* PQuery build date: " + std::string(PQBUILDDATE));
#ifdef HAVE_MYSQL
  pqLogger->addRecordToLog(
      "* PQuery MySQL client library: " + std::string(MYSQL_FORK) + " v." +
      getMySqlClientInfo());
#endif
#ifdef HAVE_PGSQL
  pqLogger->addRecordToLog("* PQuery PgSQL client library: PgSQL v." +
                           getPgSqlClientInfo());
#endif
  pqLogger->addRecordToLog("* Pquery master with PID " +
                           std::to_string(getpid()) + " started at " +
                           std::string(std::ctime(&start_time)));
}

bool PQuery::initConfig() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  configReader = std::make_shared<INIReader>(configFilePath);
  int parseerr = configReader->ParseError();

  if (parseerr < 0) {
    std::cerr << "Can't load config from file \"" + configFilePath + "\""
              << '\n';
    return false;
  }
  if (parseerr > 0) {
    std::cerr << "Config parse error!" << '\n';
    std::cerr << "File: " << configFilePath << '\n';
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
  logVersionInfo();
  return true;
}

void PQuery::doCleanup(std::string name) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::string logfile = configReader->Get("master", "logdir", "/tmp") + FSSEP +
                        name + "_worker.log";
  pqLogger->initLogFile(logfile);
}

void PQuery::reportConfigError(const std::string &secName,
                               const std::string &msg) {
  std::string fullMsg = "=> Config error in section [" + secName + "]: " + msg;
  if (pqLogger) {
    pqLogger->addRecordToLog(fullMsg);
  }
  std::cerr << fullMsg << '\n';
}

bool PQuery::checkIntRange(const std::string &secName, const std::string &name,
                           std::int64_t value, std::int64_t min,
                           std::int64_t max) {
  if (value < min || value > max) {
    reportConfigError(secName, name + " = " + std::to_string(value) +
                                   " out of range [" + std::to_string(min) +
                                   ".." + std::to_string(max) + "]");
    return false;
  }
  return true;
}

bool PQuery::setupWorkerParams(struct workerParams &wParams,
                               std::string secName) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  wParams.myName = secName;
  wParams.address = configReader->Get(secName, "address", "localhost");
  wParams.username = configReader->Get(secName, "user", "test");
  wParams.password = configReader->Get(secName, "password", "");
  wParams.socket =
      configReader->Get(secName, "socket", "/var/run/mysqld/mysql.sock");
  wParams.database = configReader->Get(secName, "database", "test");

  wParams.dbtype = configReader->getDbType(secName, "dbtype", eMYSQL);

  if (wParams.dbtype == eNONE) {
    reportConfigError(secName, "invalid or missing dbtype");
    return false;
  }

#ifndef HAVE_MYSQL
  if (wParams.dbtype == eMYSQL) {
    reportConfigError(secName, "pquery is not compiled with support for " +
                                   dbtype_str(wParams.dbtype));
    return false;
  }
#endif
#ifndef HAVE_PGSQL
  if (wParams.dbtype == ePGSQL) {
    reportConfigError(secName, "pquery is not compiled with support for " +
                                   dbtype_str(wParams.dbtype));
    return false;
  }
#endif

  try {
    switch (wParams.dbtype) {
      case eMYSQL: {
        std::int64_t port = configReader->GetInteger(secName, "port", 3306);
        if (!checkIntRange(secName, "port", port, 1, 65535)) return false;
        wParams.port = static_cast<std::uint16_t>(port);
        break;
      }
      case ePGSQL: {
        std::int64_t port = configReader->GetInteger(secName, "port", 5432);
        if (!checkIntRange(secName, "port", port, 1, 65535)) return false;
        wParams.port = static_cast<std::uint16_t>(port);
        break;
      }
      default:
        wParams.port = 0;
        break;
    }

    std::int64_t threads = configReader->GetInteger(secName, "threads", 10);
    if (!checkIntRange(secName, "threads", threads, 1, 65535)) return false;
    wParams.threads = static_cast<std::uint16_t>(threads);

    std::int64_t queries_per_thread =
        configReader->GetInteger(secName, "queries-per-thread", 10000);
    if (!checkIntRange(secName, "queries-per-thread", queries_per_thread, 0,
                       std::numeric_limits<std::int64_t>::max()))
      return false;
    wParams.queries_per_thread = static_cast<std::uint64_t>(queries_per_thread);

    std::int64_t query_list_maxsize =
        configReader->GetInteger(secName, "query-list-maxsize", 1073741824);
    if (!checkIntRange(secName, "query-list-maxsize", query_list_maxsize, 0,
                       std::numeric_limits<std::int64_t>::max()))
      return false;
    wParams.query_list_maxsize = static_cast<std::uint64_t>(query_list_maxsize);

    std::int64_t timeout_secs = configReader->GetInteger(secName, "timeout", 0);
    if (!checkIntRange(secName, "timeout", timeout_secs, 0,
                       std::numeric_limits<std::uint32_t>::max()))
      return false;
    wParams.timeout_secs = static_cast<std::uint32_t>(timeout_secs);

    std::int64_t connect_timeout_secs =
        configReader->GetInteger(secName, "connect-timeout", 60);
    if (!checkIntRange(secName, "connect-timeout", connect_timeout_secs, 0,
                       std::numeric_limits<std::uint32_t>::max()))
      return false;
    wParams.connect_timeout_secs =
        static_cast<std::uint32_t>(connect_timeout_secs);
  } catch (const std::exception &e) {
    reportConfigError(secName, e.what());
    return false;
  }

  wParams.verbose = configReader->GetBoolean(secName, "verbose", false);
  wParams.shuffle = configReader->GetBoolean(secName, "shuffle", false);

  wParams.infile = configReader->Get(secName, "infile", "pquery.sql");
  wParams.infiletype = configReader->getInfileType(secName, "infiletype", eSQL);

  if (wParams.infiletype == eUNKNOWN) {
    reportConfigError(secName, "invalid infiletype value");
    return false;
  }

  if (wParams.infiletype == eGENLOG || wParams.infiletype == eBINLOG) {
    if (wParams.dbtype == ePGSQL) {
      reportConfigError(
          secName,
          "infiletype GENLOG/BINLOG refers to MySQL log formats and is not "
          "applicable to PostgreSQL; use infiletype = SQL");
    } else {
      reportConfigError(
          secName,
          "infiletype GENLOG/BINLOG is not implemented yet, only SQL is "
          "supported");
    }
    return false;
  }

  wParams.logdir = configReader->Get(secName, "logdir", "/tmp");
  //
  wParams.log_all_queries =
      configReader->GetBoolean(secName, "log-all-queries", false);
  wParams.log_succeeded_queries =
      configReader->GetBoolean(secName, "log-succeded-queries", false);
  wParams.log_failed_queries =
      configReader->GetBoolean(secName, "log-failed-queries", false);
  wParams.log_query_statistics =
      configReader->GetBoolean(secName, "log-query-statistics", false);
  wParams.log_query_duration =
      configReader->GetBoolean(secName, "log-query-duration", false);
  wParams.log_client_output =
      configReader->GetBoolean(secName, "log-client-output", false);
  wParams.log_query_numbers =
      configReader->GetBoolean(secName, "log-query-numbers", false);

  return true;
}

void PQuery::logWorkerDetails(struct workerParams &Params) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  pqLogger->addSeparation('#', 40);
  pqLogger->addRecordToLog("## Config name: " + Params.myName);
  pqLogger->addRecordToLog("## DB type: " + dbtype_str(Params.dbtype));
  pqLogger->addRecordToLog("## DB name: " + Params.database);
  pqLogger->addRecordToLog("## DB address: " + Params.address);
  pqLogger->addRecordToLog("## DB username: " + Params.username);
  pqLogger->addRecordToLog(
      "## DB password: " +
      std::string(Params.password.empty() ? "<empty>" : "<redacted>"));
  pqLogger->addRecordToLog("## DB socket: " + Params.socket);
  pqLogger->addRecordToLog("## DB port: " + std::to_string(Params.port));
  pqLogger->addRecordToLog("## PQuery threads: " +
                           std::to_string(Params.threads));
  pqLogger->addRecordToLog("## PQuery queries per thread: " +
                           std::to_string(Params.queries_per_thread));
  pqLogger->addRecordToLog("## PQuery verbosity: " +
                           std::to_string(static_cast<int>(Params.verbose)));
  pqLogger->addRecordToLog("## PQuery shuffle: " +
                           std::to_string(static_cast<int>(Params.shuffle)));
  pqLogger->addRecordToLog("## PQuery infile: " + Params.infile);
  pqLogger->addRecordToLog("## PQuery infile type: " +
                           infiletype_str(Params.infiletype));
  pqLogger->addRecordToLog("## PQuery maxsize for query list: " +
                           std::to_string(Params.query_list_maxsize));
  pqLogger->addRecordToLog("## PQuery log directory: " + Params.logdir);
  pqLogger->addRecordToLog(
      "## PQuery log all queries: " +
      std::to_string(static_cast<int>(Params.log_all_queries)));
  pqLogger->addRecordToLog(
      "## PQuery log failed queries: " +
      std::to_string(static_cast<int>(Params.log_failed_queries)));
  pqLogger->addRecordToLog(
      "## PQuery log query statistics: " +
      std::to_string(static_cast<int>(Params.log_query_statistics)));
  pqLogger->addRecordToLog(
      "## PQuery log query duration: " +
      std::to_string(static_cast<int>(Params.log_query_duration)));
  pqLogger->addRecordToLog(
      "## PQuery log client output: " +
      std::to_string(static_cast<int>(Params.log_client_output)));
  pqLogger->addRecordToLog(
      "## PQuery log query numbers: " +
      std::to_string(static_cast<int>(Params.log_query_numbers)));
  pqLogger->addSeparation('#', 40);
}

eRETCODE PQuery::createWorkerProcess(struct workerParams &Params) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  pqLogger->flushLog();
  pid_t childPID = 0;
  childPID = fork();

  if (childPID < 0) {
    pqLogger->addRecordToLog("=> Cannot fork() child process: " +
                             std::string(std::strerror(errno)));
    return eERROR;
  }

  if (childPID > 0) {
    pqLogger->addRecordToLog("-> Waiting for created worker " +
                             std::to_string(childPID));

    workerProc proc;
    proc.pid = childPID;
    proc.name = Params.myName;
    proc.has_timeout = Params.timeout_secs > 0;
    if (proc.has_timeout) {
      proc.deadline = std::chrono::steady_clock::now() +
                      std::chrono::seconds(Params.timeout_secs);
    }
    proc.term_sent = false;
    activeWorkers.push_back(proc);

    return eMASTER;
  }

  if (childPID == 0) {
    doCleanup(Params.myName);

    if (Params.verbose) {
      logWorkerDetails(Params);
    }

    switch (Params.dbtype) {
#ifdef HAVE_MYSQL
      case eMYSQL:
        dbWorker = std::make_shared<MysqlWorker>();
        break;
#endif
#ifdef HAVE_PGSQL
      case ePGSQL:
        dbWorker = std::make_shared<PgsqlWorker>();
        break;
#endif
      default:
        std::cerr << "=> Unable to create worker of unsupported type "
                  << dbtype_str(Params.dbtype) << '\n';
        pqLogger->addRecordToLog("=> PQuery is not compiled with " +
                                 dbtype_str(Params.dbtype));
        return eERROR;
    }

    // TODO
    bool success = false;
    dbWorker->setupLogger(pqLogger);
    success = dbWorker->executeTests(Params);

    if (!success) {
      return eERROR;
    }

    return eCHILD;  // fake
  }
  return eDEFAULT;
}

bool PQuery::waitForWorkers() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  bool retvalue = true;
  const auto SIGTERM_GRACE = std::chrono::seconds(10);

  while (!activeWorkers.empty()) {
    int status = 0;
    pid_t wPID = waitpid(-1, &status, WNOHANG);

    if (wPID > 0) {
      auto it = std::find_if(
          activeWorkers.begin(), activeWorkers.end(),
          [wPID](const workerProc &proc) { return proc.pid == wPID; });
      if (it != activeWorkers.end()) {
        if (status != 0) {
          retvalue = false;
        }
        pqLogger->addRecordToLog("=> Exit status of child with PID " +
                                 std::to_string(wPID) + ": " +
                                 std::to_string(status));
        activeWorkers.erase(it);
      }
      continue;  // check for more exited children before sleeping
    }

    if (wPID < 0) {
      // No more children to wait for (e.g. ECHILD); avoid a busy loop.
      break;
    }

    // wPID == 0: nobody exited yet, check timeouts.
    auto now = std::chrono::steady_clock::now();
    for (auto &proc : activeWorkers) {
      if (proc.has_timeout && !proc.term_sent && now > proc.deadline) {
        pqLogger->addRecordToLog("=> Worker " + proc.name + " (PID " +
                                 std::to_string(proc.pid) +
                                 ") exceeded timeout, sending SIGTERM");
        kill(proc.pid, SIGTERM);
        proc.term_sent = true;
        proc.kill_deadline = now + SIGTERM_GRACE;
        retvalue = false;
      } else if (proc.term_sent && now > proc.kill_deadline) {
        pqLogger->addRecordToLog(
            "=> Worker " + proc.name + " (PID " + std::to_string(proc.pid) +
            ") did not respond to SIGTERM, sending SIGKILL");
        kill(proc.pid, SIGKILL);
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return retvalue;
}

bool PQuery::runWorkers() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::vector<std::string> sections;
  sections = configReader->GetSections();
  std::vector<std::string>::iterator it;
  std::vector<struct workerParams> validParams;
  bool configOk = true;

  // Phase 1: validate every section before forking anything, so a bad
  // config in one section can never leave sibling workers running.
  for (it = sections.begin(); it != sections.end(); it++) {
    const std::string &secName = *it;
    if (toLowerCase(secName) == "master") {
      continue;
    }
    pqLogger->addRecordToLog("-> Checking " + secName + " params...");
    if (!configReader->GetBoolean(secName, "run", false)) {
      continue;
    }
    struct workerParams wParams;
    if (!setupWorkerParams(wParams, secName)) {
      configOk = false;
      continue;
    }
    validParams.push_back(wParams);
  }  // for()

  if (!configOk) {
    const std::string msg = "=> Config validation failed, no workers started";
    pqLogger->addRecordToLog(msg);
    std::cerr << msg << '\n';
    return false;
  }

  // Phase 2: all sections validated, now fork the workers.
  for (auto &wParams : validParams) {
    pqLogger->addRecordToLog("-> Running worker for " + wParams.myName);
    eRETCODE wrc = createWorkerProcess(wParams);
    switch (wrc) {
      case eERROR:
        return false;
      case eCHILD:
        return true;
      default:
        break;
    }
  }

  return waitForWorkers();
}

int PQuery::run() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  showVersion();
  if (!prepareToRun()) {
    return EXIT_FAILURE;
  }
  if (!runWorkers()) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void PQuery::showVersion() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::cout << "* PQuery version: " << PQVERSION << '\n';
  std::cout << "* PQuery revision: " << PQREVISION << '\n';
  std::cout << "* PQuery release date: " << PQRELDATE << '\n';
  std::cout << "* PQuery build date: " << PQBUILDDATE << '\n';
#ifdef HAVE_MYSQL
  std::cout << "* PQuery MySQL client library: " + std::string(MYSQL_FORK) +
                   " v." + getMySqlClientInfo()
            << std::endl;
#endif
#ifdef HAVE_PGSQL
  std::cout << "* PQuery PgSQL client library: PgSQL v." + getPgSqlClientInfo()
            << std::endl;
#endif
}

void PQuery::showHelp() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::cout << " - Usage: pquery --config-file=pquery.cfg" << '\n';
  std::cout << " - CLI params has been replaced by config file (INI format)"
            << '\n';
  std::cout << " - You can redefine any global param=value pair in "
               "host-specific section"
            << '\n';
  std::cout << "\n# Config example:\n\n"
            << "# \"#\" and \";\" are comments here\n"
            << "############################\n"
            << "# Section for master process\n"
            << "[master]\n\n"
            << "# Directory to store logs\n"
            << "logdir = /tmp\n\n"
            << "# Logfile for master process\n"
            << "logfile = pquery3-master.log\n\n"
            << "############################\n"
            << "[node0.domain.tld]\n\n"
            << "# The database to connect to\n"
            << "database = test\n\n"
            << "# Database type (MySQL, PostgreSQL), default is MySQL\n"
            << "dbtype = MySQL\n\n"
            << "# IP address to connect to, default is AF_UNIX\n"
            << "address = <empty>\n\n"
            << "# The port to connect to\n"
            << "port = 3306\n\n"
            << "# The SQL input file\n"
            << "infile = pquery.sql\n\n"
            << "# Infile type: SQL only (GENLOG and BINLOG are not "
               "implemented yet)\n"
            << "infiletype = SQL\n\n"
            << "# Maximum file size to be loaded to RAM\n"
            << "query-list-maxsize = 1G\n\n"
            << "# Overall timeout in seconds for this worker, 0 = no timeout\n"
               "timeout = 0\n\n"
            << "# Connection timeout in seconds\n"
               "connect-timeout = 60\n\n"
            << "# Directory to store logs\n"
            << "logdir = /tmp\n\n"
            << "# Socket file to use\n"
            << "socket = /tmp/my.sock\n\n"
            << "# The DB userID to be used\n"
            << "user = test\n\n"
            << "# The DB user's password\n"
            << "password = test\n\n"
            << "# The number of threads to use by worker\n"
            << "threads = 1\n\n"
            << "# The number of queries per thread\n"
               "queries-per-thread = 10000\n\n"
            << "# Duplicates the log to console when threads=1 and workers=1\n"
               "verbose = No\n\n"
            << "# Log all queries\n"
            << "log-all-queries = No\n\n"
            << "# Log succeeded queries\n"
            << "log-succeeded-queries = No\n\n"
            << "# Log failed queries\n"
            << "log-failed-queries = No\n\n"
            << "# Execute SQL randomly or sequentially\n"
            << "shuffle = No\n\n"
            << "# Extended output of query result\n"
               "log-query-statistics = No\n\n"
            << "# Log query duration in milliseconds\n"
            << "log-query-duration = No\n\n"
            << "# Log output from executed query (separate log)\n"
            << "log-client-output = No\n\n"
            << "# Log query numbers along the query results and statistics\n"
            << "log-query-numbers = No\n\n"
            << "# default for \"run\" is No, need to set it to YES explicitly\n"
            << "run = Yes\n\n"
            << "############################\n"
            << "[node1.domain.tld]\n"
            << "address = 10.10.6.11\n"
            << '\n';
}

bool PQuery::parseCliOptions(int argc, char *argv[]) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  int c = 0;
  while (true) {
    static struct option long_options[] = {
        // config file with all options
        {"config-file", required_argument, nullptr, 'c'},
        {"master-logfile", required_argument, nullptr, 'L'},
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'v'},
        // finally
        {nullptr, 0, nullptr, 0}};
    int option_index = 0;
    c = getopt_long_only(argc, argv, "c:L:hv", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
      case 'c':
        setConfigFilePath(optarg);
        break;
      case 'L':
        setLogFilePath(optarg);
        break;
      case 'h':
        showHelp();
        return false;
      case 'v':
        showVersion();
        return false;
      default:
        break;
    }
  }
  return true;
}
