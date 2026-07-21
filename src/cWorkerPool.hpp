#include <sys/types.h>
#include <chrono>
#include <eTypes.hpp>
#include <memory>
#include <sWorkerParams.hpp>
#include <string>
#include <vector>

#ifndef CWORKERPOOL_HPP
#define CWORKERPOOL_HPP

class Logger;

class WorkerPool {
 public:
  WorkerPool(std::shared_ptr<Logger> logger, std::string masterLogDir);
  bool runAll(std::vector<workerParams> &params);

 private:
  struct workerProc {
    pid_t pid;
    std::string name;
    bool has_timeout;
    std::chrono::steady_clock::time_point deadline;
    bool term_sent;
    std::chrono::steady_clock::time_point kill_deadline;
  };
  eRETCODE spawnWorker(workerParams &params);
  bool waitForWorkers();
  void reinitChildLog(const std::string &name);
  void logWorkerDetails(const workerParams &params);

  std::vector<workerProc> activeWorkers;
  std::shared_ptr<Logger> logger;
  std::string masterLogDir;
};
#endif
