#include <libpq-fe.h>
#include <cDatabase.hpp>

#ifndef _PGSQLDATABASE_
#define _PGSQLDATABASE_

class PgsqlDatabase : public Database {
 public:
  PgsqlDatabase();
  ~PgsqlDatabase() override;
  std::string getServerVersion() override;
  std::string getHostInfo() override;
  std::string getErrorString() override;
  bool connect(const workerParams &) override;
  std::uint64_t getAffectedRows() override;
  bool performRealQuery(const std::string &) override;
  void processQueryOutput() override;
  std::uint32_t getWarningsCount() override;
  void cleanupResult() override;

 private:
  PGconn *conn;
  PGresult *res;
  ExecStatusType pgstatus;
};
#endif
