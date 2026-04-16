#include <iostream>
#include <hCommon.hpp>
#include <cMysqlWorker.hpp>
#include <cMysqlDatabase.hpp>
/*
 */
#include <cSqlFileParser.hpp>
#include <cMyGenLogParser.hpp>
#include <cMyBinLogParser.hpp>
/*
 */

MysqlWorker::MysqlWorker() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  if (mysql_library_init(0, NULL, NULL)) {
    throw std::runtime_error("=> Could not initialize MySQL client library");
    }
  }


MysqlWorker::~MysqlWorker() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  mysql_library_end();
  }


bool
MysqlWorker::testConnection() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif

  std::shared_ptr<Database> mysqlDB = createDbInstance();

  if(!mysqlDB->connect(mParams)) {
    wLogger->addRecordToLog("=> Unable to connect! MySQL error " + mysqlDB->getErrorString());
    return false;
    }

  wLogger->addRecordToLog("-> Successfully connected to " + mysqlDB->getHostInfo());
  wLogger->addRecordToLog("-> Server version: " + mysqlDB->getServerVersion());
  return true;

  }


bool
MysqlWorker::loadQueryList() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif

  switch (mParams.infiletype) {
    case eSQL:
      wInfileParser = std::make_shared<SqlFileParser>();
      break;
    case eGENLOG:
      wInfileParser = std::make_shared<MyGenLogParser>();
      break;
    case eBINLOG:
      wInfileParser = std::make_shared<MyBinLogParser>();
      break;
    default:
      std::cerr << "=> Unsupported infile type: " << infiletype_str(mParams.infiletype) << std::endl;
      return false;
    }

  if (!wInfileParser) {
    std::cerr << "=> Unable to create infile parser" << std::endl;
    return false;
    }

  std::uint64_t file_size = wInfileParser->getInfileSize(mParams.infile);

#ifdef DEBUG
  std::cerr << "=> Infile type: " << infiletype_str(mParams.infiletype) << std::endl;
  std::cerr << "=> Infile size: " << file_size << " Bytes" << std::endl;
  std::cerr << "=> Infile max RAM: " << mParams.query_list_maxsize << " bytes" << std::endl;
#endif

  if (file_size == 0) {
    std::cerr << "=> Unable to read infile size or file is empty: " << mParams.infile << std::endl;
    return false;
    }

  if (file_size > mParams.query_list_maxsize) {
    std::cerr << "=> Unable to load file " << mParams.infile
      << " to RAM due to limit " << mParams.query_list_maxsize << " bytes" << std::endl;
    std::cerr << "=> InFile size: " << file_size << " bytes..." << std::endl;
    return false;
    }

  if (!wInfileParser->loadQueriesFromFile(queryList, mParams.infile)) {
    return false;
    }

  return true;
  }


std::shared_ptr<Database>
MysqlWorker::createDbInstance() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::shared_ptr<Database> mysqlDB = std::make_shared<MysqlDatabase>();
  return mysqlDB;
  }


void
MysqlWorker::endDbThread() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  mysql_thread_end();
  }
