#include <iostream>
#include <sstream>
#include <cPgsqlDatabase.hpp>

PgsqlDatabase::PgsqlDatabase() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  conn = NULL;
  res = NULL;
  }


PgsqlDatabase::~PgsqlDatabase() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  if(conn != NULL) { PQfinish(conn); }
  if(res != NULL) { PQclear(res); }
  }


void
PgsqlDatabase::processQueryOutput() {
  if(res == NULL) { return; }

  queryResult.clear();
  int rows = PQntuples(res);
  int columns = PQnfields(res);
  for(int row=0; row<rows; row++) {
    for(int col=0; col<columns; col++) {
      std::string strres = PQgetvalue(res, row, col);
      if (!strres.empty()) {
        queryResult += strres;
        queryResult += " ";
        }
      else {
        queryResult += "#EMPTY# ";
        }
      }
    queryResult += "\n";
    }
  }


bool
PgsqlDatabase::connect(workerParams& dbParams) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::ostringstream conninfo;
  conninfo << "host=" << dbParams.address << " user=" << dbParams.username << " password=" << dbParams.password
    << " dbname=" << dbParams.database << " port=" << dbParams.port;
  conn = PQconnectdb(conninfo.str().c_str());
  if (PQstatus(conn) != CONNECTION_OK) { return false; }
  return true;
  }


bool
PgsqlDatabase::performRealQuery(std::string query) {
  res = PQexec(conn, query.c_str());
  pgstatus = PQresultStatus(res);
  return (pgstatus == PGRES_TUPLES_OK) ||
    (pgstatus == PGRES_COMMAND_OK);
  }


std::uint32_t
PgsqlDatabase::getWarningsCount() {
  return 0;
  }


std::string
PgsqlDatabase::getServerVersion() {
  std::string server_version;
  res = PQexec(conn, "SELECT VERSION()");
  if (PQresultStatus(res) == PGRES_TUPLES_OK) {
    server_version = PQgetvalue(res, 0, 0);
    }
  else {
    server_version = "PostgreSQL Server " + std::to_string(PQserverVersion(conn));
    }
  cleanupResult();
  return server_version;
  }


std::string
PgsqlDatabase::getErrorString() {
  std::string psql_errstring = PQerrorMessage(conn);
  if(psql_errstring.substr(0, 5) == "ERROR") {
    psql_errstring = psql_errstring.substr(5, std::string::npos);
    }
  std::size_t found = psql_errstring.find_first_of("\n");
  if(found == std::string::npos) {
    return psql_errstring;
    }
  return PQresStatus(pgstatus) + psql_errstring.substr(0, found);
  }


std::string
PgsqlDatabase::getHostInfo() {
  std::string host_info = PQhost(conn);
  host_info += " port ";
  host_info += PQport(conn);
  return host_info;
  }


inline std::uint64_t
PgsqlDatabase::getAffectedRows() {
  std::string affected_rows = PQcmdTuples(res);
  if(affected_rows.empty()){ return 0; }
  return std::stoll(affected_rows);
  }


void
PgsqlDatabase::cleanupResult() {
  if (res != NULL) {
    PQclear(res);
    res = NULL;
    }
  }
