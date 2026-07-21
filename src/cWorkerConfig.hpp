#include <cstdint>
#include <memory>
#include <sWorkerParams.hpp>
#include <string>
#include <vector>

#ifndef CWORKERCONFIG_HPP
#define CWORKERCONFIG_HPP

class INIReader;
class Logger;

class WorkerConfig {
 public:
  WorkerConfig(std::shared_ptr<INIReader> reader,
               std::shared_ptr<Logger> logger);
  bool buildAllWorkerParams(std::vector<workerParams> &out);
  bool setupWorkerParams(workerParams &params, const std::string &secName);

 private:
  void reportConfigError(const std::string &secName, const std::string &msg);
  bool checkIntRange(const std::string &secName, const std::string &name,
                     std::int64_t value, std::int64_t min, std::int64_t max);
  std::shared_ptr<INIReader> reader;
  std::shared_ptr<Logger> logger;
};
#endif
