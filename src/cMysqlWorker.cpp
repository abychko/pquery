#include <cMysqlWorker.hpp>

#include <cMysqlDatabase.hpp>
#include <cSqlFileParser.hpp>
#include <cMyGenLogParser.hpp>
#include <cMyBinLogParser.hpp>
#include <hCommon.hpp>

#include <iostream>
#include <memory>

MysqlWorker::MysqlWorker() {
  }


MysqlWorker::~MysqlWorker() {
  }


void
MysqlWorker::endDbThread() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  mysql_thread_end();
  }


bool
MysqlWorker::testConnection() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif

  std::shared_ptr<Database> db = createDbInstance();
  if (!db) {
    std::cerr << "=> Unable to create database instance" << std::endl;
    return false;
    }

  if (!db->connect(mParams)) {
    std::cerr << "=> Unable to connect to database: " << db->getErrorString() << std::endl;
    return false;
    }

  return true;
  }


std::shared_ptr<InfileParser>
MysqlWorker::createInfileParser() const
  {
  switch (mParams.infiletype) {
    case eSQL:
      return std::make_shared<SqlFileParser>();
    case eGENLOG:
      return std::make_shared<MyGenLogParser>();
    case eBINLOG:
      return std::make_shared<MyBinLogParser>();
    default:
      std::cerr << "=> Unsupported infile type: " << infiletype_str(mParams.infiletype) << std::endl;
      return nullptr;
    }
  }


bool
MysqlWorker::validateInfileSize(const InfileParser& parser) const
  {
  std::uint64_t file_size = parser.getInfileSize(mParams.infile);

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

  return true;
  }


bool
MysqlWorker::loadQueries(InfileParser& parser) {
  return parser.loadQueriesFromFile(queryList, mParams.infile);
  }


bool
MysqlWorker::loadQueryList() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif

  wInfileParser = createInfileParser();
  if (!wInfileParser) {
    std::cerr << "=> Unable to create infile parser" << std::endl;
    return false;
    }

  if (!validateInfileSize(*wInfileParser)) {
    return false;
    }

  if (!loadQueries(*wInfileParser)) {
    return false;
    }

  return true;
  }


std::shared_ptr<Database>
MysqlWorker::createDbInstance() {
  return std::make_shared<MysqlDatabase>();
  }
