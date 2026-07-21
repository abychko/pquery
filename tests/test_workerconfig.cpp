// test_workerconfig.cpp
//
// Lightweight, assert-style unit tests for WorkerConfig (no external test
// framework, run through CTest). Exercises WorkerConfig::setupWorkerParams()
// and WorkerConfig::buildAllWorkerParams(), which were extracted out of the
// former PQuery god-object without changing behavior.

#include <cIniReader.hpp>
#include <cLogger.hpp>
#include <cWorkerConfig.hpp>
#include <eTypes.hpp>
#include <sWorkerParams.hpp>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {
int g_checks = 0;
int g_failures = 0;

void reportCheck(bool cond, const char *expr, const char *file, int line) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "FAIL: " << file << ":" << line << ": " << expr << std::endl;
  }
}

#define CHECK(cond) reportCheck((cond), #cond, __FILE__, __LINE__)

std::string writeTempFile(const std::string &name, const std::string &content) {
  std::string path = "workerconfig_" + name + ".tmp.cfg";
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << content;
  out.close();
  return path;
}

void removeFile(const std::string &path) { std::remove(path.c_str()); }

std::shared_ptr<INIReader> makeReader(const std::string &name,
                                      const std::string &content) {
  std::string path = writeTempFile(name, content);
  auto reader = std::make_shared<INIReader>(path);
  removeFile(path);
  return reader;
}

// Reads back the full contents of a log file written by a real Logger, so
// tests can assert on the exact error text reportConfigError() produced.
std::string readLogFile(const std::string &path) {
  std::ifstream in(path);
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

std::shared_ptr<Logger> makeFileLogger(const std::string &name) {
  std::string path = "workerconfig_" + name + ".tmp.log";
  removeFile(path);
  auto logger = std::make_shared<Logger>();
  logger->initLogFile(path);
  return logger;
}

void testValidMySqlSectionDefaults() {
  auto reader = makeReader("valid_mysql",
                           "[node0]\n"
                           "dbtype = MySQL\n"
                           "database = test\n"
                           "run = yes\n");
  WorkerConfig config(reader, nullptr);
  workerParams params;
  bool ok = config.setupWorkerParams(params, "node0");

  CHECK(ok);
  CHECK(params.myName == "node0");
  CHECK(params.dbtype == eMYSQL);
  CHECK(params.port == 3306);
  CHECK(params.threads == 10);
  CHECK(params.queries_per_thread == 10000);
  CHECK(params.query_list_maxsize == 1073741824ULL);
  CHECK(params.connect_timeout_secs == 60);
  CHECK(params.timeout_secs == 0);
  CHECK(params.infiletype == eSQL);
  CHECK(params.address == "localhost");
  CHECK(params.username == "test");
  CHECK(params.database == "test");
  CHECK(!params.verbose);
  CHECK(!params.shuffle);
}

void testValidPgSqlSectionDefaults() {
  auto reader = makeReader("valid_pgsql",
                           "[node0]\n"
                           "dbtype = PostgreSQL\n"
                           "database = test\n"
                           "run = yes\n");
  WorkerConfig config(reader, nullptr);
  workerParams params;
  bool ok = config.setupWorkerParams(params, "node0");

  CHECK(ok);
  CHECK(params.dbtype == ePGSQL);
  CHECK(params.port == 5432);
  CHECK(params.threads == 10);
  CHECK(params.queries_per_thread == 10000);
  CHECK(params.connect_timeout_secs == 60);
  CHECK(params.timeout_secs == 0);
}

void testPortOutOfRange() {
  {
    auto reader =
        makeReader("port_zero", "[node0]\ndbtype = MySQL\nport = 0\n");
    WorkerConfig config(reader, nullptr);
    workerParams params;
    CHECK(!config.setupWorkerParams(params, "node0"));
  }
  {
    auto reader =
        makeReader("port_toobig", "[node0]\ndbtype = MySQL\nport = 65536\n");
    WorkerConfig config(reader, nullptr);
    workerParams params;
    CHECK(!config.setupWorkerParams(params, "node0"));
  }
}

void testThreadsOutOfRange() {
  {
    auto reader =
        makeReader("threads_zero", "[node0]\ndbtype = MySQL\nthreads = 0\n");
    WorkerConfig config(reader, nullptr);
    workerParams params;
    CHECK(!config.setupWorkerParams(params, "node0"));
  }
  {
    auto reader = makeReader("threads_toobig",
                             "[node0]\ndbtype = MySQL\nthreads = 65536\n");
    WorkerConfig config(reader, nullptr);
    workerParams params;
    CHECK(!config.setupWorkerParams(params, "node0"));
  }
}

void testQueriesPerThreadNegative() {
  auto reader = makeReader(
      "queries_negative", "[node0]\ndbtype = MySQL\nqueries-per-thread = -1\n");
  WorkerConfig config(reader, nullptr);
  workerParams params;
  CHECK(!config.setupWorkerParams(params, "node0"));
}

void testTimeoutAboveUint32Max() {
  auto reader = makeReader("timeout_toobig",
                           "[node0]\ndbtype = MySQL\ntimeout = 4294967296\n");
  WorkerConfig config(reader, nullptr);
  workerParams params;
  CHECK(!config.setupWorkerParams(params, "node0"));
}

void testInvalidDbType() {
  auto reader = makeReader("invalid_dbtype", "[node0]\ndbtype = oracle\n");
  WorkerConfig config(reader, nullptr);
  workerParams params;
  CHECK(!config.setupWorkerParams(params, "node0"));
}

void testInvalidInfileType() {
  auto reader = makeReader("invalid_infiletype",
                           "[node0]\ndbtype = MySQL\ninfiletype = xml\n");
  WorkerConfig config(reader, nullptr);
  workerParams params;
  CHECK(!config.setupWorkerParams(params, "node0"));
}

void testGenlogNotImplementedForMysql() {
  auto reader = makeReader("genlog_mysql",
                           "[node0]\ndbtype = MySQL\ninfiletype = GENLOG\n");
  auto logger = makeFileLogger("genlog_mysql");
  WorkerConfig config(reader, logger);
  workerParams params;
  bool ok = config.setupWorkerParams(params, "node0");
  logger->flushLog();

  CHECK(!ok);
  std::string logContent = readLogFile("workerconfig_genlog_mysql.tmp.log");
  CHECK(logContent.find("not implemented yet") != std::string::npos);
  removeFile("workerconfig_genlog_mysql.tmp.log");
}

void testGenlogNotApplicableForPgsql() {
  auto reader = makeReader(
      "genlog_pgsql", "[node0]\ndbtype = PostgreSQL\ninfiletype = GENLOG\n");
  auto logger = makeFileLogger("genlog_pgsql");
  WorkerConfig config(reader, logger);
  workerParams params;
  bool ok = config.setupWorkerParams(params, "node0");
  logger->flushLog();

  CHECK(!ok);
  std::string logContent = readLogFile("workerconfig_genlog_pgsql.tmp.log");
  CHECK(logContent.find("not applicable to PostgreSQL") != std::string::npos);
  removeFile("workerconfig_genlog_pgsql.tmp.log");
}

void testNonNumericThreadsDoesNotThrow() {
  auto reader = makeReader("threads_nonnumeric",
                           "[node0]\ndbtype = MySQL\nthreads = abc\n");
  WorkerConfig config(reader, nullptr);
  workerParams params;
  bool ok = false;
  try {
    ok = config.setupWorkerParams(params, "node0");
  } catch (...) {
    CHECK(false);  // must not propagate an exception
  }
  CHECK(!ok);
}

void testBuildAllWorkerParamsSkipsMasterAndCollectsValid() {
  auto reader = makeReader("build_valid",
                           "[master]\n"
                           "logdir = /tmp\n"
                           "\n"
                           "[node0]\n"
                           "dbtype = MySQL\n"
                           "run = yes\n");
  WorkerConfig config(reader, nullptr);
  std::vector<workerParams> out;
  bool ok = config.buildAllWorkerParams(out);

  CHECK(ok);
  CHECK(out.size() == 1);
  if (out.size() == 1) {
    CHECK(out[0].myName == "node0");
  }
}

void testBuildAllWorkerParamsSkipsRunNo() {
  auto reader = makeReader("build_run_no",
                           "[node0]\n"
                           "dbtype = MySQL\n"
                           "run = no\n");
  WorkerConfig config(reader, nullptr);
  std::vector<workerParams> out;
  bool ok = config.buildAllWorkerParams(out);

  CHECK(ok);
  CHECK(out.empty());
}

void testBuildAllWorkerParamsFailsFastOnAnyInvalidSection() {
  auto reader = makeReader("build_mixed",
                           "[node0]\n"
                           "dbtype = MySQL\n"
                           "run = yes\n"
                           "\n"
                           "[node1]\n"
                           "dbtype = oracle\n"
                           "run = yes\n");
  WorkerConfig config(reader, nullptr);
  std::vector<workerParams> out;
  bool ok = config.buildAllWorkerParams(out);

  // Matches PQuery::runWorkers()'s original two-phase behavior: sibling
  // sections keep getting validated (so a bad section can't hide behind an
  // earlier good one), but as soon as any section is invalid the overall
  // result must be false so that no workers ever get forked.
  CHECK(!ok);
}

}  // namespace

int main() {
  testValidMySqlSectionDefaults();
  testValidPgSqlSectionDefaults();
  testPortOutOfRange();
  testThreadsOutOfRange();
  testQueriesPerThreadNegative();
  testTimeoutAboveUint32Max();
  testInvalidDbType();
  testInvalidInfileType();
  testGenlogNotImplementedForMysql();
  testGenlogNotApplicableForPgsql();
  testNonNumericThreadsDoesNotThrow();
  testBuildAllWorkerParamsSkipsMasterAndCollectsValid();
  testBuildAllWorkerParamsSkipsRunNo();
  testBuildAllWorkerParamsFailsFastOnAnyInvalidSection();

  std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed"
            << std::endl;

  return g_failures == 0 ? 0 : 1;
}
