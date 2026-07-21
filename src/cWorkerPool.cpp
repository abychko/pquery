#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cDbWorker.hpp>
#include <cLogger.hpp>
#include <cWorkerPool.hpp>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <hCommon.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
//
#ifdef HAVE_MYSQL
#include <cMysqlWorker.hpp>
#endif

#ifdef HAVE_PGSQL
#include <cPgsqlWorker.hpp>
#endif

WorkerPool::WorkerPool(std::shared_ptr<Logger> logger, std::string masterLogDir)
    : logger(logger), masterLogDir(masterLogDir) {}

void WorkerPool::reinitChildLog(const std::string &name) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::string logfile = masterLogDir + FSSEP + name + "_worker.log";
  logger->initLogFile(logfile);
}

void WorkerPool::logWorkerDetails(const workerParams &Params) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  logger->addSeparation('#', 40);
  logger->addRecordToLog("## Config name: " + Params.myName);
  logger->addRecordToLog("## DB type: " + dbtype_str(Params.dbtype));
  logger->addRecordToLog("## DB name: " + Params.database);
  logger->addRecordToLog("## DB address: " + Params.address);
  logger->addRecordToLog("## DB username: " + Params.username);
  logger->addRecordToLog(
      "## DB password: " +
      std::string(Params.password.empty() ? "<empty>" : "<redacted>"));
  logger->addRecordToLog("## DB socket: " + Params.socket);
  logger->addRecordToLog("## DB port: " + std::to_string(Params.port));
  logger->addRecordToLog("## PQuery threads: " +
                         std::to_string(Params.threads));
  logger->addRecordToLog("## PQuery queries per thread: " +
                         std::to_string(Params.queries_per_thread));
  logger->addRecordToLog("## PQuery verbosity: " +
                         std::to_string(static_cast<int>(Params.verbose)));
  logger->addRecordToLog("## PQuery shuffle: " +
                         std::to_string(static_cast<int>(Params.shuffle)));
  logger->addRecordToLog("## PQuery infile: " + Params.infile);
  logger->addRecordToLog("## PQuery infile type: " +
                         infiletype_str(Params.infiletype));
  logger->addRecordToLog("## PQuery maxsize for query list: " +
                         std::to_string(Params.query_list_maxsize));
  logger->addRecordToLog("## PQuery log directory: " + Params.logdir);
  logger->addRecordToLog(
      "## PQuery log all queries: " +
      std::to_string(static_cast<int>(Params.log_all_queries)));
  logger->addRecordToLog(
      "## PQuery log failed queries: " +
      std::to_string(static_cast<int>(Params.log_failed_queries)));
  logger->addRecordToLog(
      "## PQuery log query statistics: " +
      std::to_string(static_cast<int>(Params.log_query_statistics)));
  logger->addRecordToLog(
      "## PQuery log query duration: " +
      std::to_string(static_cast<int>(Params.log_query_duration)));
  logger->addRecordToLog(
      "## PQuery log client output: " +
      std::to_string(static_cast<int>(Params.log_client_output)));
  logger->addRecordToLog(
      "## PQuery log query numbers: " +
      std::to_string(static_cast<int>(Params.log_query_numbers)));
  logger->addSeparation('#', 40);
}

eRETCODE WorkerPool::spawnWorker(struct workerParams &Params) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  logger->flushLog();
  pid_t childPID = 0;
  childPID = fork();

  if (childPID < 0) {
    logger->addRecordToLog("=> Cannot fork() child process: " +
                           std::string(std::strerror(errno)));
    return eERROR;
  }

  if (childPID > 0) {
    logger->addRecordToLog("-> Waiting for created worker " +
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
    reinitChildLog(Params.myName);

    if (Params.verbose) {
      logWorkerDetails(Params);
    }

    std::shared_ptr<DbWorker> dbWorker;
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
        logger->addRecordToLog("=> PQuery is not compiled with " +
                               dbtype_str(Params.dbtype));
        return eERROR;
    }

    // TODO
    bool success = false;
    dbWorker->setupLogger(logger);
    success = dbWorker->executeTests(Params);

    if (!success) {
      return eERROR;
    }

    return eCHILD;  // fake
  }
  return eDEFAULT;
}

bool WorkerPool::waitForWorkers() {
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
        logger->addRecordToLog("=> Exit status of child with PID " +
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
        logger->addRecordToLog("=> Worker " + proc.name + " (PID " +
                               std::to_string(proc.pid) +
                               ") exceeded timeout, sending SIGTERM");
        kill(proc.pid, SIGTERM);
        proc.term_sent = true;
        proc.kill_deadline = now + SIGTERM_GRACE;
        retvalue = false;
      } else if (proc.term_sent && now > proc.kill_deadline) {
        logger->addRecordToLog("=> Worker " + proc.name + " (PID " +
                               std::to_string(proc.pid) +
                               ") did not respond to SIGTERM, sending SIGKILL");
        kill(proc.pid, SIGKILL);
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return retvalue;
}

bool WorkerPool::runAll(std::vector<workerParams> &params) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  for (auto &wParams : params) {
    logger->addRecordToLog("-> Running worker for " + wParams.myName);
    eRETCODE wrc = spawnWorker(wParams);
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
