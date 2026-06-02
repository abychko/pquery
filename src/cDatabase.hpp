#include <chrono>
#include <cstdint>
#include <sWorkerParams.hpp>
#include <string>

#ifndef CDATABASE_HPP
#define CDATABASE_HPP

class Database {
 public:
  Database();
  virtual ~Database();

  virtual std::string getServerVersion() = 0;
  virtual std::string getHostInfo() = 0;
  virtual std::string getErrorString() = 0;
  virtual bool connect(const workerParams &) = 0;
  virtual std::uint64_t getAffectedRows() = 0;
  virtual bool performRealQuery(const std::string &query) = 0;
  virtual void processQueryOutput() = 0;
  virtual std::uint32_t getWarningsCount() = 0;
  virtual void cleanupResult() = 0;

  double getQueryDurationMs() const;
  std::uint64_t getPerformedQueries() const { return performed_queries; }
  std::uint64_t getFailedQueries() const { return failed_queries; }
  std::uint16_t getConsecutiveFailures() const { return consecutive_failures; }
  bool performQuery(const std::string &query);
  const std::string &getQueryResult() const { return queryResult; }

 protected:
  std::string queryResult;

 private:
  std::chrono::steady_clock::time_point begin;
  std::chrono::steady_clock::time_point end;
  std::uint64_t failed_queries;
  std::uint64_t performed_queries;
  std::uint16_t consecutive_failures;
};
#endif
