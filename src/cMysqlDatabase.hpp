#include <mysql.h>
#include <cDatabase.hpp>

#ifndef _MYSQLDATABASE_
#define _MYSQLDATABASE_

class MysqlDatabase : public Database {
 public:
  MysqlDatabase();
  ~MysqlDatabase() override;

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
  MYSQL *conn;
};
#endif
