#include <cMysqlDatabase.hpp>
#include <cstring>
#include <iostream>

MysqlDatabase::MysqlDatabase() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  conn = nullptr;
}

MysqlDatabase::~MysqlDatabase() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  if (conn != nullptr) {
    mysql_close(conn);
  }
}

std::uint64_t MysqlDatabase::getAffectedRows() {
  my_ulonglong affected = mysql_affected_rows(conn);
  if (affected == static_cast<my_ulonglong>(-1)) {
    return 0;
  }
  return static_cast<std::uint64_t>(affected);
}

bool MysqlDatabase::connect(const workerParams &dbParams) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  if (conn != nullptr) {
    mysql_close(conn);
    conn = nullptr;
  }

  conn = mysql_init(nullptr);
  if (conn == nullptr) {
    return false;
  }

  if (dbParams.connect_timeout_secs > 0) {
    unsigned int timeout = dbParams.connect_timeout_secs;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
  }

#ifdef DEBUG
  std::cerr << "\n"
            << "=> Connecting to:\n"
            << "=> IP Address: " << dbParams.address << "\n"
            << "=> Socket: " << dbParams.socket << "\n"
            << std::endl;
#endif

  if (mysql_real_connect(conn, dbParams.address.c_str(),
                         dbParams.username.c_str(), dbParams.password.c_str(),
                         dbParams.database.c_str(), dbParams.port,
                         dbParams.socket.c_str(), 0) == nullptr) {
    return false;
  }

  return true;
}

bool MysqlDatabase::performRealQuery(const std::string &query) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  return mysql_real_query(conn, query.c_str(), (unsigned long)query.length()) ==
         0;
}

std::uint32_t MysqlDatabase::getWarningsCount() {
  return mysql_warning_count(conn);
}

void MysqlDatabase::processQueryOutput() {
  queryResult.clear();

  do {
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == nullptr) {
      if (mysql_field_count(conn) == 0) {
        continue;
      }
      // A result set was expected but mysql_store_result() failed (query
      // error). Drain any remaining multi-statement results so the
      // connection isn't left out of sync ("commands out of sync") for the
      // next query.
      while (mysql_next_result(conn) == 0) {
        MYSQL_RES *pending = mysql_store_result(conn);
        if (pending != nullptr) {
          mysql_free_result(pending);
        }
      }
      return;
    }

    MYSQL_ROW row;
    unsigned int columns = mysql_num_fields(result);

    while ((row = mysql_fetch_row(result)) != nullptr) {
      unsigned long *lengths = mysql_fetch_lengths(result);

      for (unsigned int i = 0; i < columns; i++) {
        if (i > 0) {
          queryResult += "\t";
        }

        if (row[i] == nullptr) {
          queryResult += "NULL";
        } else if (lengths[i] == 0) {
          queryResult += "EMPTY";
        } else {
          queryResult.append(row[i], lengths[i]);
        }
      }

      queryResult += "\n";
    }

    mysql_free_result(result);
  } while (mysql_next_result(conn) == 0);
}

std::string MysqlDatabase::getHostInfo() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  return mysql_get_host_info(conn);
}

std::string MysqlDatabase::getErrorString() {
  if (conn == nullptr) {
    return "MySQL connection is not initialized";
  }

  return std::to_string(mysql_errno(conn)) + ": " + mysql_error(conn);
}

std::string MysqlDatabase::getServerVersion() {
  if (conn == nullptr) {
    return "";
  }

  std::string server_version = mysql_get_server_info(conn);

  if (mysql_query(conn, "select @@version_comment limit 1") == 0) {
    MYSQL_RES *result = mysql_store_result(conn);
    if (result != nullptr) {
      MYSQL_ROW row = mysql_fetch_row(result);
      if (row != nullptr && row[0] != nullptr) {
        server_version += " ";
        server_version += row[0];
      }
      mysql_free_result(result);
    }
  }

  return server_version;
}

void MysqlDatabase::cleanupResult() {}
