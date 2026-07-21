#include <getopt.h>
#include <unistd.h>
#include <cCli.hpp>
#include <cLogger.hpp>
#include <chrono>
#include <ctime>
#include <hCommon.hpp>
#include <iostream>
#include <string>
//
#ifdef HAVE_MYSQL
#include <mysql.h>
#endif

#ifdef HAVE_PGSQL
#include <pg_config.h>
#endif

#ifdef HAVE_MYSQL
static std::string getMySqlClientInfo() { return mysql_get_client_info(); }
#endif

#ifdef HAVE_PGSQL
static std::string getPgSqlClientInfo() { return std::string(PG_VERSION); }
#endif

void showVersion() {
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

void showHelp() {
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

void logVersionInfo(Logger &logger) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  auto now = std::chrono::system_clock::now();
  std::time_t start_time = std::chrono::system_clock::to_time_t(now);
  logger.addRecordToLog("* PQuery version: " + std::string(PQVERSION));
  logger.addRecordToLog("* PQuery revision: " + std::string(PQREVISION));
  logger.addRecordToLog("* PQuery revision date: " + std::string(PQRELDATE));
  logger.addRecordToLog("* PQuery build date: " + std::string(PQBUILDDATE));
#ifdef HAVE_MYSQL
  logger.addRecordToLog("* PQuery MySQL client library: " +
                        std::string(MYSQL_FORK) + " v." + getMySqlClientInfo());
#endif
#ifdef HAVE_PGSQL
  logger.addRecordToLog("* PQuery PgSQL client library: PgSQL v." +
                        getPgSqlClientInfo());
#endif
  logger.addRecordToLog("* Pquery master with PID " + std::to_string(getpid()) +
                        " started at " + std::string(std::ctime(&start_time)));
}

bool parseCliOptions(int argc, char *argv[], CliOptions &opts) {
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
        opts.configFilePath = optarg;
        break;
      case 'L':
        opts.logFilePath = optarg;
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
