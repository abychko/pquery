// sWorkerParams.hpp
#include <cstdint>
#include <eTypes.hpp>
#include <string>

#ifndef SWORKERPARAMS_HPP
#define SWORKERPARAMS_HPP

struct workerParams {
  std::string myName{};  // unique name for worker
  std::string database{};
  eDBTYPE dbtype{eNONE};
  std::string address{};
  std::string socket{};
  std::string username{};
  std::string password{};
  std::string infile{};
  eINFILETYPE infiletype{eUNKNOWN};
  std::string logdir{};
  std::uint16_t port{0};
  std::uint16_t threads{0};
  std::uint64_t queries_per_thread{0};
  std::uint64_t query_list_maxsize{0};  // max memory for query list. will not
                                        // be loaded to memory if greater
  bool verbose{false};
  bool log_all_queries{false};
  bool log_failed_queries{false};
  bool log_succeeded_queries{false};
  bool log_query_statistics{false};
  bool log_query_duration{false};
  bool log_client_output{false};
  bool log_query_numbers{false};
  bool shuffle{false};
  std::uint32_t timeout_secs{0};  // 0 = no timeout for the whole worker run
  std::uint32_t connect_timeout_secs{60};  // connection timeout, seconds
};

#endif
