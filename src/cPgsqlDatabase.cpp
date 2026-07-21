#include <cPgsqlDatabase.hpp>
#include <iostream>
#include <sstream>

PgsqlDatabase::PgsqlDatabase() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  conn = nullptr;
}

PgsqlDatabase::~PgsqlDatabase() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  if (conn != nullptr) {
    PQfinish(conn);
  }
}

void PgsqlDatabase::processQueryOutput() {
  if (!result_.hasResult()) {
    return;
  }

  queryResult.clear();
  int rows = PQntuples(result_.get());
  int columns = PQnfields(result_.get());
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < columns; col++) {
      std::string strres = PQgetvalue(result_.get(), row, col);
      if (!strres.empty()) {
        queryResult += strres;
        queryResult += " ";
      } else {
        queryResult += "#EMPTY# ";
      }
    }
    queryResult += "\n";
  }
}

bool PgsqlDatabase::connect(const workerParams &dbParams) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::ostringstream conninfo;
  conninfo << "host=" << dbParams.address << " user=" << dbParams.username
           << " password=" << dbParams.password
           << " dbname=" << dbParams.database << " port=" << dbParams.port;
  if (dbParams.connect_timeout_secs > 0) {
    conninfo << " connect_timeout=" << dbParams.connect_timeout_secs;
  }
  conn = PQconnectdb(conninfo.str().c_str());
  if (PQstatus(conn) != CONNECTION_OK) {
    return false;
  }
  return true;
}

bool PgsqlDatabase::performRealQuery(const std::string &query) {
  result_ = PgResult(PQexec(conn, query.c_str()));
  return (result_.status() == PGRES_TUPLES_OK) ||
         (result_.status() == PGRES_COMMAND_OK);
}

std::uint32_t PgsqlDatabase::getWarningsCount() { return 0; }

std::string PgsqlDatabase::getServerVersion() {
  std::string server_version;
  PgResult versionResult(PQexec(conn, "SELECT VERSION()"));
  if (versionResult.status() == PGRES_TUPLES_OK) {
    server_version = PQgetvalue(versionResult.get(), 0, 0);
  } else {
    server_version =
        "PostgreSQL Server " + std::to_string(PQserverVersion(conn));
  }
  return server_version;
}

std::string PgsqlDatabase::getErrorString() {
  std::string psql_errstring = PQerrorMessage(conn);
  if (psql_errstring.substr(0, 5) == "ERROR") {
    psql_errstring = psql_errstring.substr(5, std::string::npos);
  }
  std::size_t found = psql_errstring.find_first_of("\n");
  if (found == std::string::npos) {
    return psql_errstring;
  }
  if (result_.hasResult()) {
    return std::string(PQresStatus(result_.status())) +
           psql_errstring.substr(0, found);
  }
  return psql_errstring.substr(0, found);
}

std::string PgsqlDatabase::getHostInfo() {
  std::string host_info = PQhost(conn);
  host_info += " port ";
  host_info += PQport(conn);
  return host_info;
}

std::uint64_t PgsqlDatabase::getAffectedRows() {
  if (!result_.hasResult()) {
    return 0;
  }
  std::string affected_rows = PQcmdTuples(result_.get());
  if (affected_rows.empty()) {
    return 0;
  }
  return std::stoll(affected_rows);
}

void PgsqlDatabase::cleanupResult() { result_.reset(); }
