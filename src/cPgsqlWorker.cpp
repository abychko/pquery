#include <cPgsqlDatabase.hpp>
#include <cPgsqlWorker.hpp>
#include <cSqlFileParser.hpp>
#include <iostream>

PgsqlWorker::PgsqlWorker() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
}

PgsqlWorker::~PgsqlWorker() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
}

bool PgsqlWorker::testConnection() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif

  std::shared_ptr<Database> pgsqlDB = createDbInstance();

  if (!pgsqlDB->connect(mParams)) {
    wLogger->addRecordToLog("=> Unable to connect! PgSQL error " +
                            pgsqlDB->getErrorString());
    return false;
  }

  wLogger->addRecordToLog("-> Successfully connected to " +
                          pgsqlDB->getHostInfo());
  wLogger->addRecordToLog("-> Server version: " + pgsqlDB->getServerVersion());
  return true;
}

std::shared_ptr<Database> PgsqlWorker::createDbInstance() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::shared_ptr<Database> pgsqlDB = std::make_shared<PgsqlDatabase>();
  return pgsqlDB;
}

void PgsqlWorker::endDbThread() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
}

bool PgsqlWorker::loadQueryList() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif

  wInfileParser = std::make_shared<SqlFileParser>();
  if (!wInfileParser) {
    std::cerr << "=> Unable to create infile parser" << std::endl;
    return false;
  }

  std::uint64_t file_size = wInfileParser->getInfileSize(mParams.infile);
  if (file_size == 0) {
    std::cerr << "=> Unable to read infile size or file is empty: "
              << mParams.infile << std::endl;
    return false;
  }

  if (file_size > mParams.query_list_maxsize) {
    std::cerr << "=> Unable to load file " << mParams.infile
              << " to RAM due to limit " << mParams.query_list_maxsize
              << " bytes" << std::endl;
    return false;
  }

  return wInfileParser->loadQueriesFromFile(queryList, mParams.infile);
}
