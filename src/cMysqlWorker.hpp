#ifndef C_MYSQLWORKER_HPP
#define C_MYSQLWORKER_HPP

#include <cDbWorker.hpp>
#include <memory>

class InfileParser;
class Database;

class MysqlWorker : public DbWorker {
 public:
  MysqlWorker();
  ~MysqlWorker() override;

  bool testConnection() override;
  std::shared_ptr<Database> createDbInstance() override;
  void endDbThread() override;

 protected:
  std::shared_ptr<InfileParser> createInfileParser() const override;
};
#endif
