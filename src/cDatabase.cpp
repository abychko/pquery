#ifdef DEBUG
#include <iostream>
#endif
#include <cDatabase.hpp>

Database::Database() {
  failed_queries = 0;
  performed_queries = 0;
  consecutive_failures = 0;
  }


Database::~Database() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  }


double
Database::getQueryDurationMs() const
  {
  std::chrono::duration<double, std::milli> duration = end - begin;
  return duration.count();
  }


bool
Database::performQuery(const std::string& query) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  queryResult.clear();

  begin = std::chrono::steady_clock::now();
  bool success = performRealQuery(query);
  end = std::chrono::steady_clock::now();

  if (success) {
    consecutive_failures = 0;
    }
  else {
    failed_queries++;
    consecutive_failures++;
    }

  performed_queries++;
  return success;
  }
