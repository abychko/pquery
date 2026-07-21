// cDbWorker.hpp
#include <atomic>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <cDatabase.hpp>
#include <cInfileParser.hpp>
#include <cLogger.hpp>
#include <sWorkerParams.hpp>

#ifndef PQDBWORKER_HPP
#define PQDBWORKER_HPP

const std::uint16_t MAX_CON_FAILURES = 250;

class DbWorker {
 public:
  DbWorker();
  virtual ~DbWorker();
  bool executeTests(struct workerParams &);
  void setupLogger(std::shared_ptr<Logger>);
  virtual std::shared_ptr<Database> createDbInstance() = 0;
  virtual void endDbThread() = 0;
  virtual bool loadQueryList();

 protected:
  // Hook for subclasses that support infile types beyond plain SQL (e.g.
  // MysqlWorker adds GENLOG/BINLOG parsing). The default implementation
  // only knows about eSQL.
  virtual std::shared_ptr<InfileParser> createInfileParser() const;
  void workerThread(int);
  void adjustRuntimeParams();
  void spawnWorkerThreads();
  std::vector<std::thread> workers;
  std::shared_ptr<Logger> wLogger;
  std::shared_ptr<InfileParser> wInfileParser;
  std::shared_ptr<std::vector<std::string>> queryList;
  struct workerParams mParams;
  std::uint64_t getFailedConnectionsTotal() const {
    return failed_connections_total.load();
  }

 private:
  void writeFinalReport();
  void calculateQueries(std::shared_ptr<Database>);
  virtual bool testConnection() = 0;
  void storeParams(struct workerParams &wParams);
  bool validateInfileSize(const InfileParser &parser) const;
  bool loadQueries(InfileParser &parser);
  std::atomic<uint64_t> performed_queries_total{};
  std::atomic<uint64_t> failed_queries_total{};
  std::atomic<std::uint64_t> failed_connections_total{0};
  std::atomic<bool> thread_failed{false};
};
#endif
