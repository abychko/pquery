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
  bool loadQueryList() override;
  std::shared_ptr<Database> createDbInstance() override;
  void endDbThread() override;

 private:
  std::shared_ptr<InfileParser> createInfileParser() const;
  bool validateInfileSize(const InfileParser &parser) const;
  bool loadQueries(InfileParser &parser);
};
#endif
