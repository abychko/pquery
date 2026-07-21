#include <libpq-fe.h>
#include <cDatabase.hpp>

#ifndef _PGSQLDATABASE_
#define _PGSQLDATABASE_

// Owns exactly one PGresult and the ExecStatusType captured when it was
// acquired, so the pointer and its status can never drift apart.
class PgResult {
 public:
  PgResult() = default;
  explicit PgResult(PGresult *r)
      : res_(r), status_(r ? PQresultStatus(r) : PGRES_EMPTY_QUERY) {}
  ~PgResult() { reset(); }
  PgResult(const PgResult &) = delete;
  PgResult &operator=(const PgResult &) = delete;
  PgResult &operator=(PgResult &&other) noexcept {
    if (this != &other) {
      reset();
      res_ = other.res_;
      status_ = other.status_;
      other.res_ = nullptr;
      other.status_ = PGRES_EMPTY_QUERY;
    }
    return *this;
  }
  void reset() {
    if (res_ != nullptr) {
      PQclear(res_);
      res_ = nullptr;
    }
    status_ = PGRES_EMPTY_QUERY;
  }
  PGresult *get() const { return res_; }
  bool hasResult() const { return res_ != nullptr; }
  ExecStatusType status() const { return status_; }

 private:
  PGresult *res_ = nullptr;
  ExecStatusType status_ = PGRES_EMPTY_QUERY;
};

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
  PgResult result_;
};
#endif
