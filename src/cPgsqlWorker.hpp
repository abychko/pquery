#include <cDbWorker.hpp>

#ifndef PQPGSQLWORKER_HPP
#define PQPGSQLWORKER_HPP

class PgsqlWorker : public DbWorker {
 public:
  PgsqlWorker();
  ~PgsqlWorker() override;
  std::shared_ptr<Database> createDbInstance() override;
  void endDbThread() override;
  bool loadQueryList() override;

 private:
  bool testConnection() override;
};
#endif
