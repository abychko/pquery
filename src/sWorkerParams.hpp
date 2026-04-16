// sWorkerParams.hpp
#include <string>
#include <eTypes.hpp>

#ifndef _SWORKER_PARAMS_
#define _SWORKER_PARAMS_

struct
workerParams
  {
  std::string myName;                             // unique name for worker
  std::string database;
  eDBTYPE dbtype;
  std::string address;
  std::string socket;
  std::string username;
  std::string password;
  std::string infile;
  eINFILETYPE infiletype;
  std::string logdir;
  uint16_t port;
  uint16_t threads;
  uint64_t queries_per_thread;
  uint64_t query_list_maxsize;                    // max memory for query list. will not be loaded to memory if greater
  bool verbose;
  bool log_all_queries;
  bool log_failed_queries;
  bool log_succeeded_queries;
  bool log_query_statistics;
  bool log_query_duration;
  bool log_client_output;
  bool log_query_numbers;
  bool shuffle;
  };
#endif
