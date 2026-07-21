#include <cMysqlWorker.hpp>

#include <cMyBinLogParser.hpp>
#include <cMyGenLogParser.hpp>
#include <cMysqlDatabase.hpp>
#include <cSqlFileParser.hpp>
#include <hCommon.hpp>

#include <iostream>
#include <memory>

MysqlWorker::MysqlWorker() {}

MysqlWorker::~MysqlWorker() {}

void MysqlWorker::endDbThread() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  mysql_thread_end();
}

bool MysqlWorker::testConnection() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << '\n';
#endif

  std::shared_ptr<Database> db = createDbInstance();
  if (!db) {
    std::cerr << "=> Unable to create database instance\n";
    return false;
  }

  if (!db->connect(mParams)) {
    std::string target = mParams.address;

    if (!target.empty() && mParams.port != 0) {
      target += ":" + std::to_string(mParams.port);
    } else if (target.empty() && !mParams.socket.empty()) {
      target = "socket " + mParams.socket;
    }

    std::cerr << "\n=> Unable to connect to host " << target << "\n"
              << "=> " << db->getErrorString() << '\n';
    return false;
  }

  return true;
}

std::shared_ptr<InfileParser> MysqlWorker::createInfileParser() const {
  switch (mParams.infiletype) {
    case eSQL:
      return std::make_shared<SqlFileParser>();
    case eGENLOG:
      return std::make_shared<MyGenLogParser>();
    case eBINLOG:
      return std::make_shared<MyBinLogParser>();
    default:
      std::cerr << "=> Unsupported infile type: "
                << infiletype_str(mParams.infiletype) << std::endl;
      return nullptr;
  }
}

std::shared_ptr<Database> MysqlWorker::createDbInstance() {
  return std::make_shared<MysqlDatabase>();
}
