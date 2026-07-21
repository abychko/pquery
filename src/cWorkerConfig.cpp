#include <cIniReader.hpp>
#include <cLogger.hpp>
#include <cWorkerConfig.hpp>
#include <cstdint>
#include <hCommon.hpp>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include "eTypes.hpp"

WorkerConfig::WorkerConfig(std::shared_ptr<INIReader> reader,
                           std::shared_ptr<Logger> logger)
    : reader(reader), logger(logger) {}

void WorkerConfig::reportConfigError(const std::string &secName,
                                     const std::string &msg) {
  std::string fullMsg = "=> Config error in section [" + secName + "]: " + msg;
  if (logger) {
    logger->addRecordToLog(fullMsg);
  }
  std::cerr << fullMsg << '\n';
}

bool WorkerConfig::checkIntRange(const std::string &secName,
                                 const std::string &name, std::int64_t value,
                                 std::int64_t min, std::int64_t max) {
  if (value < min || value > max) {
    reportConfigError(secName, name + " = " + std::to_string(value) +
                                   " out of range [" + std::to_string(min) +
                                   ".." + std::to_string(max) + "]");
    return false;
  }
  return true;
}

bool WorkerConfig::setupWorkerParams(struct workerParams &wParams,
                                     const std::string &secName) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  wParams.myName = secName;
  wParams.address = reader->Get(secName, "address", "localhost");
  wParams.username = reader->Get(secName, "user", "test");
  wParams.password = reader->Get(secName, "password", "");
  wParams.socket = reader->Get(secName, "socket", "/var/run/mysqld/mysql.sock");
  wParams.database = reader->Get(secName, "database", "test");

  wParams.dbtype = reader->getDbType(secName, "dbtype", eMYSQL);

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
        std::int64_t port = reader->GetInteger(secName, "port", 3306);
        if (!checkIntRange(secName, "port", port, 1, 65535)) return false;
        wParams.port = static_cast<std::uint16_t>(port);
        break;
      }
      case ePGSQL: {
        std::int64_t port = reader->GetInteger(secName, "port", 5432);
        if (!checkIntRange(secName, "port", port, 1, 65535)) return false;
        wParams.port = static_cast<std::uint16_t>(port);
        break;
      }
      default:
        wParams.port = 0;
        break;
    }

    std::int64_t threads = reader->GetInteger(secName, "threads", 10);
    if (!checkIntRange(secName, "threads", threads, 1, 65535)) return false;
    wParams.threads = static_cast<std::uint16_t>(threads);

    std::int64_t queries_per_thread =
        reader->GetInteger(secName, "queries-per-thread", 10000);
    if (!checkIntRange(secName, "queries-per-thread", queries_per_thread, 0,
                       std::numeric_limits<std::int64_t>::max()))
      return false;
    wParams.queries_per_thread = static_cast<std::uint64_t>(queries_per_thread);

    std::int64_t query_list_maxsize =
        reader->GetInteger(secName, "query-list-maxsize", 1073741824);
    if (!checkIntRange(secName, "query-list-maxsize", query_list_maxsize, 0,
                       std::numeric_limits<std::int64_t>::max()))
      return false;
    wParams.query_list_maxsize = static_cast<std::uint64_t>(query_list_maxsize);

    std::int64_t timeout_secs = reader->GetInteger(secName, "timeout", 0);
    if (!checkIntRange(secName, "timeout", timeout_secs, 0,
                       std::numeric_limits<std::uint32_t>::max()))
      return false;
    wParams.timeout_secs = static_cast<std::uint32_t>(timeout_secs);

    std::int64_t connect_timeout_secs =
        reader->GetInteger(secName, "connect-timeout", 60);
    if (!checkIntRange(secName, "connect-timeout", connect_timeout_secs, 0,
                       std::numeric_limits<std::uint32_t>::max()))
      return false;
    wParams.connect_timeout_secs =
        static_cast<std::uint32_t>(connect_timeout_secs);
  } catch (const std::exception &e) {
    reportConfigError(secName, e.what());
    return false;
  }

  wParams.verbose = reader->GetBoolean(secName, "verbose", false);
  wParams.shuffle = reader->GetBoolean(secName, "shuffle", false);

  wParams.infile = reader->Get(secName, "infile", "pquery.sql");
  wParams.infiletype = reader->getInfileType(secName, "infiletype", eSQL);

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

  wParams.logdir = reader->Get(secName, "logdir", "/tmp");
  //
  wParams.log_all_queries =
      reader->GetBoolean(secName, "log-all-queries", false);
  wParams.log_succeeded_queries =
      reader->GetBoolean(secName, "log-succeded-queries", false);
  wParams.log_failed_queries =
      reader->GetBoolean(secName, "log-failed-queries", false);
  wParams.log_query_statistics =
      reader->GetBoolean(secName, "log-query-statistics", false);
  wParams.log_query_duration =
      reader->GetBoolean(secName, "log-query-duration", false);
  wParams.log_client_output =
      reader->GetBoolean(secName, "log-client-output", false);
  wParams.log_query_numbers =
      reader->GetBoolean(secName, "log-query-numbers", false);

  return true;
}

bool WorkerConfig::buildAllWorkerParams(std::vector<workerParams> &out) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::vector<std::string> sections = reader->GetSections();
  bool configOk = true;

  // Phase 1: validate every section before forking anything, so a bad
  // config in one section can never leave sibling workers running.
  for (auto it = sections.begin(); it != sections.end(); it++) {
    const std::string &secName = *it;
    if (toLowerCase(secName) == "master") {
      continue;
    }
    if (logger) {
      logger->addRecordToLog("-> Checking " + secName + " params...");
    }
    if (!reader->GetBoolean(secName, "run", false)) {
      continue;
    }
    struct workerParams wParams;
    if (!setupWorkerParams(wParams, secName)) {
      configOk = false;
      continue;
    }
    out.push_back(wParams);
  }  // for()

  if (!configOk) {
    const std::string msg = "=> Config validation failed, no workers started";
    if (logger) {
      logger->addRecordToLog(msg);
    }
    std::cerr << msg << '\n';
    return false;
  }

  return true;
}
