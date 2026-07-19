// cDbWorker.cpp
#include <cDbWorker.hpp>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

DbWorker::DbWorker() : performed_queries_total(0), failed_queries_total(0) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  workers.clear();
}

DbWorker::~DbWorker() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void DbWorker::adjustRuntimeParams() {
  /* log replaying */
  if (!mParams.shuffle) {
    wLogger->addRecordToLog(
        "-> Setting # of threads to 1 for log replaying due to \"shuffle = "
        "False\"");
    mParams.threads = 1;
    *wLogger << "-> Setting queries per thread to " << queryList->size()
             << " due to \"shuffle = False\" (the size of query list)\n";
    mParams.queries_per_thread = queryList->size();
  }
  /* END log replaying */
}

void DbWorker::writeFinalReport() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::ostringstream exitmsg;
  exitmsg.precision(2);
  exitmsg << std::fixed;
  exitmsg << "-> WORKER SUMMARY: " << failed_queries_total << "/"
          << performed_queries_total << " queries failed, (";
  if (performed_queries_total == 0) {
    exitmsg << "N/A";
  } else {
    exitmsg << (performed_queries_total - failed_queries_total) * 100.0 /
                   performed_queries_total;
  }
  exitmsg << "% were successful)";
  wLogger->addRecordToLog(exitmsg.str());
}

void DbWorker::storeParams(struct workerParams &wParams) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  mParams = wParams;
}

void DbWorker::setupLogger(std::shared_ptr<Logger> logger) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  wLogger = logger;
}

bool DbWorker::isComment(std::string &line) {
  size_t first = line.find_first_not_of(' ');
  if (std::string::npos == first) {
    return false;
  }
  size_t last = line.find_last_not_of(' ');
  auto qStr = line.substr(first, (last - first + 1));
  return ((qStr.rfind('#', 0) == 0) || (qStr.rfind(';', 0) == 0) ||
          (qStr.rfind("//", 0) == 0) || (qStr.rfind("--", 0) == 0));
}

void DbWorker::calculateQueries(std::shared_ptr<Database> Database) {
  performed_queries_total += Database->getPerformedQueries();
  failed_queries_total += Database->getFailedQueries();
}

void DbWorker::workerThread(int number) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << " " << number << std::endl;
#endif
  std::shared_ptr<Logger> threadLogger = nullptr;
  std::shared_ptr<Logger> outputLogger = nullptr;

  if ((!queryList) || (queryList->empty())) {
    if (wLogger) {
      wLogger->addRecordToLog("=> Thread #" + std::to_string(number) +
                              " is exiting abnormally, query list is empty");
    }
    return;
  }


  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(0, queryList->size() - 1);

  if (mParams.log_client_output) {
    outputLogger = std::make_shared<Logger>();
    outputLogger->initLogFile(mParams.logdir + "/" + mParams.myName +
                              "_thread-" + std::to_string(number) + ".out");
  }

  if ((mParams.log_failed_queries) || (mParams.log_all_queries) ||
      (mParams.log_succeeded_queries) || (mParams.log_query_duration) ||
      (mParams.log_query_statistics) || (mParams.log_query_numbers)) {
    threadLogger = std::make_shared<Logger>();
    threadLogger->initLogFile(mParams.logdir + "/" + mParams.myName +
                              "_thread-" + std::to_string(number) + ".sql");
  }

  std::shared_ptr<Database> Database = createDbInstance();

  if (!Database->connect(mParams)) {
    if (threadLogger) {
      threadLogger->addRecordToLog("=> Unable to connect! Database error " +
                                   Database->getErrorString());
    } else if (wLogger) {
      wLogger->addRecordToLog("=> Unable to connect! Database error " +
                              Database->getErrorString());
    }

    if (wLogger) {
      *wLogger << "==> Thread #" << number
               << " is exiting abnormally, unable to init database"
               << "\n";
    }

    return;
  }

  std::uint32_t i = 0;
  std::uint32_t query_number = 0;
  for (i = 0; i < mParams.queries_per_thread; i++) {
    if (mParams.shuffle) {
      query_number = dis(gen);
    } else {
      query_number = i;
    }
    bool success = Database->performQuery((*queryList)[query_number]);
    std::uint16_t max_con_fail_count = Database->getConsecutiveFailures();
    if (max_con_fail_count >= MAX_CON_FAILURES) {
      std::ostringstream failmsg;
      failmsg << "=> Last " << max_con_fail_count
              << " consecutive queries all failed. Likely crash/assert, "
                 "user privileges drop, or similar. Ending run.";
      if (threadLogger) {
        *threadLogger << failmsg.str() << "\n";
      } else if (wLogger) {
        wLogger->addRecordToLog(failmsg.str());
      }
      calculateQueries(Database);
      return;
    }

    /*
    thread logging according to config
    */
    if (threadLogger != nullptr) {
      if ((mParams.log_all_queries) ||
          (mParams.log_succeeded_queries && success) ||
          (mParams.log_failed_queries && !success)) {
        *threadLogger << (*queryList)[query_number];
      }  // if we should log queries

      if (mParams.log_query_statistics) {
        if (success) {
          *threadLogger << "#NOERROR";
        } else {
          *threadLogger << "#ERROR ";
          *threadLogger << Database->getErrorString();
        }
        *threadLogger << "#WARNINGS: " << Database->getWarningsCount();
        *threadLogger << "#CHANGED: " << Database->getAffectedRows();
      }  // log_query_statistics
      if (mParams.log_query_duration) {
        *threadLogger << "#Duration: " << Database->getQueryDurationMs()
                      << " ms";
      }
      if (mParams.log_query_numbers) {
        *threadLogger << "#" << query_number + 1;
      }
      threadLogger->addPartialRecord("\n");
    }
    Database->processQueryOutput();
    std::string strToLog = Database->getQueryResult();
    if ((!strToLog.empty()) && (outputLogger != nullptr)) {
      *outputLogger << strToLog;
      if (mParams.log_query_numbers) {
        *outputLogger << "#" << query_number + 1;
      }
      outputLogger->addPartialRecord("\n");
    }
    Database->cleanupResult();
  }
  calculateQueries(Database);  // for (i=0; i<mParams.queries_per_thread; i++){}
  endDbThread();
}  // void DbWorker::workerThread(int number)

void DbWorker::spawnWorkerThreads() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  workers.resize(mParams.threads);
  for (int i = 0; i < mParams.threads; i++) {
    workers[i] = std::thread(&DbWorker::workerThread, this, i);
  }
  for (int i = 0; i < mParams.threads; i++) {
    workers[i].join();
  }
}

bool DbWorker::executeTests(struct workerParams &wParams) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  storeParams(wParams);

  // Ensure queryList is allocated once per worker
  if (!queryList) {
    queryList = std::make_shared<std::vector<std::string>>();
  } else {
    queryList->clear();
  }
  if (!testConnection()) {
    return false;
  }
  if (!loadQueryList()) {
    return false;
  }
  adjustRuntimeParams();
  spawnWorkerThreads();
  writeFinalReport();
  return true;
}
