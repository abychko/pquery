// test_dbworker.cpp
//
// Lightweight, assert-style unit tests for DbWorker (no external test
// framework, run through CTest). Exercises the worker-thread exception
// handling added to cDbWorker.cpp: an exception thrown inside
// DbWorker::workerThread() (e.g. from Database::connect() or
// createDbInstance()) must be caught, must not terminate the process, must
// be reflected in DbWorker::executeTests()'s return value, and must behave
// correctly under concurrent execution from multiple threads.

#include <cDatabase.hpp>
#include <cDbWorker.hpp>
#include <cInfileParser.hpp>
#include <hCommon.hpp>
#include <sWorkerParams.hpp>

#include <atomic>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
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

// A Database stub whose connect() either always throws, always fails
// (returns false without throwing), or always succeeds, depending on the
// mode requested at construction time. This lets us drive the different
// paths inside DbWorker::workerThread() without any real DB client.
class FakeDatabase : public Database {
 public:
  enum class Mode { kThrowOnConnect, kFailOnConnect, kSucceed, kFailQueries };

  explicit FakeDatabase(Mode mode, std::uint32_t succeed_every_n = 0)
      : mode_(mode), succeed_every_n_(succeed_every_n) {}

  std::string getServerVersion() override { return "fake-1.0"; }
  std::string getHostInfo() override { return "fake-host"; }
  std::string getErrorString() override { return "fake-error"; }

  bool connect(const workerParams & /*params*/) override {
    switch (mode_) {
      case Mode::kThrowOnConnect:
        throw std::runtime_error("simulated client library exception");
      case Mode::kFailOnConnect:
        return false;
      case Mode::kSucceed:
      case Mode::kFailQueries:
        return true;
    }
    return false;
  }

  std::uint64_t getAffectedRows() override { return 0; }
  bool performRealQuery(const std::string & /*query*/) override {
    if (mode_ != Mode::kFailQueries) {
      return true;
    }
    ++query_calls_;
    if (succeed_every_n_ > 0 && (query_calls_ % succeed_every_n_) == 0) {
      return true;
    }
    return false;
  }
  void processQueryOutput() override {}
  std::uint32_t getWarningsCount() override { return 0; }
  void cleanupResult() override {}

 private:
  Mode mode_;
  std::uint32_t succeed_every_n_;
  std::uint32_t query_calls_ = 0;
};

// A minimal concrete DbWorker whose factory/hook methods are configurable
// per test, so we can drive workerThread() through its various branches.
class TestDbWorker : public DbWorker {
 public:
  explicit TestDbWorker(FakeDatabase::Mode mode,
                        std::uint32_t succeed_every_n = 0)
      : mode_(mode), succeed_every_n_(succeed_every_n) {}

  std::shared_ptr<Database> createDbInstance() override {
    ++instances_created_;
    if (throw_in_factory_) {
      throw std::runtime_error("simulated factory exception");
    }
    return std::make_shared<FakeDatabase>(mode_, succeed_every_n_);
  }

  void endDbThread() override { ++threads_finished_cleanly_; }

  bool loadQueryList() override {
    queryList = std::make_shared<std::vector<std::string>>();
    queryList->push_back("SELECT 1");
    queryList->push_back("SELECT 2");
    return true;
  }

  // Exposed for the tests to drive DbWorker::executeTests() without a real
  // connection-testing step.
  bool testConnection() override { return true; }

  void setThrowInFactory(bool value) { throw_in_factory_ = value; }
  int instancesCreated() const { return instances_created_.load(); }
  int threadsFinishedCleanly() const {
    return threads_finished_cleanly_.load();
  }
  std::uint64_t failedConnectionsTotal() const {
    return getFailedConnectionsTotal();
  }
  std::uint64_t performedQueriesTotal() const {
    return getPerformedQueriesTotal();
  }
  std::uint64_t failedQueriesTotal() const { return getFailedQueriesTotal(); }

  // Exposes the protected createInfileParser() hook for the dollar-quoting
  // wiring test below: sets dbtype on mParams and returns whatever parser
  // the base class builds for it.
  std::shared_ptr<InfileParser> makeInfileParserFor(eDBTYPE dbtype) {
    mParams.dbtype = dbtype;
    mParams.infiletype = eSQL;
    return createInfileParser();
  }

 private:
  FakeDatabase::Mode mode_;
  std::uint32_t succeed_every_n_;
  bool throw_in_factory_ = false;
  std::atomic<int> instances_created_{0};
  std::atomic<int> threads_finished_cleanly_{0};
};

std::shared_ptr<Logger> makeDiscardLogger() {
  auto logger = std::make_shared<Logger>();
  // /dev/null always exists and is writable; avoids depending on cwd.
  logger->initLogFile("/dev/null");
  return logger;
}

workerParams makeParams(std::uint16_t threads) {
  workerParams params;
  params.myName = "unittest-node";
  params.threads = threads;
  params.queries_per_thread = 2;
  params.shuffle = true;   // avoid adjustRuntimeParams() forcing threads to 1
  params.logdir = "/dev";  // unused: no per-thread logging enabled below
  return params;
}

// A single thread whose connect() throws must be caught inside
// workerThread(): the process must not terminate, and executeTests() must
// report failure.
void testSingleThreadExceptionIsCaughtAndReported() {
  TestDbWorker worker(FakeDatabase::Mode::kThrowOnConnect);
  worker.setupLogger(makeDiscardLogger());
  workerParams params = makeParams(1);

  bool result = worker.executeTests(params);

  CHECK(result == false);
  CHECK(worker.instancesCreated() == 1);
  CHECK(worker.threadsFinishedCleanly() == 0);
}

// A well-behaved worker (successful connect, all queries succeed) must
// still report success, exactly like before the exception-handling change
// was introduced (regression check for the new `!thread_failed` return).
void testSuccessfulRunStillReturnsTrue() {
  TestDbWorker worker(FakeDatabase::Mode::kSucceed);
  worker.setupLogger(makeDiscardLogger());
  workerParams params = makeParams(4);

  bool result = worker.executeTests(params);

  CHECK(result == true);
  CHECK(worker.instancesCreated() == 4);
  CHECK(worker.threadsFinishedCleanly() == 4);
  CHECK(worker.failedConnectionsTotal() == 0);
}

// connect() returning false (not throwing) is a pre-existing, "expected"
// failure path (bad credentials, unreachable host, ...). It must not be
// conflated with the new exception-based thread_failed flag: executeTests()
// should still report true because the C++ exception path was never hit
// (this only checks that ordinary connection failures don't spuriously
// trip the new flag).
void testOrdinaryConnectFailureDoesNotTripExceptionFlag() {
  TestDbWorker worker(FakeDatabase::Mode::kFailOnConnect);
  worker.setupLogger(makeDiscardLogger());
  workerParams params = makeParams(3);

  bool result = worker.executeTests(params);

  CHECK(result == true);
  CHECK(worker.instancesCreated() == 3);
  CHECK(worker.threadsFinishedCleanly() == 0);
  CHECK(worker.failedConnectionsTotal() == 3);
}

// Concurrency check: many threads throwing simultaneously must not race or
// crash the process, and the shared thread_failed flag must end up set.
// This is run several times to shake out flakiness from thread scheduling.
void testManyConcurrentThreadsAllThrowingDoesNotCrash() {
  const int kThreads = 32;
  for (int iteration = 0; iteration < 5; ++iteration) {
    TestDbWorker worker(FakeDatabase::Mode::kThrowOnConnect);
    worker.setupLogger(makeDiscardLogger());
    workerParams params = makeParams(static_cast<std::uint16_t>(kThreads));

    bool result = worker.executeTests(params);

    CHECK(result == false);
    CHECK(worker.instancesCreated() == kThreads);
  }
}

// A throw from the factory method itself (createDbInstance(), which runs
// before connect()) must also be caught per-thread.
void testFactoryExceptionIsCaught() {
  TestDbWorker worker(FakeDatabase::Mode::kSucceed);
  worker.setupLogger(makeDiscardLogger());
  worker.setThrowInFactory(true);
  workerParams params = makeParams(2);

  bool result = worker.executeTests(params);

  CHECK(result == false);
}

// createInfileParser() must enable dollar-quoting (needed to keep PL/pgSQL
// function bodies like "$$ ... ; ... $$" together as one statement) only
// when dbtype is PostgreSQL. For MySQL, "$" must stay a plain identifier
// character and internal ';' inside a would-be dollar-quoted block must
// still split the statement, exactly like before this feature existed.
void testCreateInfileParserEnablesDollarQuotingOnlyForPgsql() {
  const std::string path = "dbworker_dollarquote.tmp.sql";
  {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << "CREATE FUNCTION f() RETURNS int AS $$ BEGIN SELECT 1; SELECT 2; "
           "END $$ LANGUAGE plpgsql;\n"
        << "SELECT 3;\n";
  }

  TestDbWorker pgWorker(FakeDatabase::Mode::kSucceed);
  auto pgParser = pgWorker.makeInfileParserFor(ePGSQL);
  auto pgQueries = std::make_shared<std::vector<std::string>>();
  bool pgOk = pgParser->loadQueriesFromFile(pgQueries, path);
  CHECK(pgOk);
  // The dollar-quoted function body must stay a single statement: 2
  // statements total (CREATE FUNCTION ... $$ ... $$ ...; and SELECT 3).
  CHECK(pgQueries->size() == 2);

  TestDbWorker myWorker(FakeDatabase::Mode::kSucceed);
  auto myParser = myWorker.makeInfileParserFor(eMYSQL);
  auto myQueries = std::make_shared<std::vector<std::string>>();
  bool myOk = myParser->loadQueriesFromFile(myQueries, path);
  CHECK(myOk);
  // Without dollar-quoting, the ';' characters inside the "$$ ... $$" text
  // are ordinary statement separators, so the same file splits into more
  // (smaller) statements.
  CHECK(myQueries->size() > pgQueries->size());

  std::remove(path.c_str());
}

// MAX_CON_FAILURES consecutive query failures inside a single thread must
// abort that thread's query loop early (see the `max_con_fail_count >=
// MAX_CON_FAILURES` check in workerThread()): this is a distinct, "expected"
// abort path from the exception-based thread_failed flag, so executeTests()
// must still report success, and the counters must reflect that only
// MAX_CON_FAILURES queries actually ran before bailing out. Because the
// abort path returns before calling endDbThread(), threadsFinishedCleanly()
// must stay at 0.
void testConsecutiveFailuresAbortThread() {
  TestDbWorker worker(FakeDatabase::Mode::kFailQueries);  // always fails
  worker.setupLogger(makeDiscardLogger());
  workerParams params = makeParams(1);
  params.queries_per_thread = 2 * MAX_CON_FAILURES;

  bool result = worker.executeTests(params);

  CHECK(result == true);
  CHECK(worker.threadsFinishedCleanly() == 0);
  CHECK(worker.performedQueriesTotal() == MAX_CON_FAILURES);
  CHECK(worker.failedQueriesTotal() == MAX_CON_FAILURES);
}

// Occasional successes reset the consecutive-failure counter (see
// Database::performQuery()), so the MAX_CON_FAILURES threshold should never
// be reached and the thread must run to completion normally.
void testIntermittentFailuresDoNotAbort() {
  constexpr std::uint32_t kSucceedEveryN = MAX_CON_FAILURES / 3;
  static_assert(kSucceedEveryN > 0 && kSucceedEveryN < MAX_CON_FAILURES,
                "reset must fire well before the abort threshold");

  TestDbWorker worker(FakeDatabase::Mode::kFailQueries, kSucceedEveryN);
  worker.setupLogger(makeDiscardLogger());
  workerParams params = makeParams(1);
  params.queries_per_thread = 2 * MAX_CON_FAILURES;

  bool result = worker.executeTests(params);

  CHECK(result == true);
  CHECK(worker.threadsFinishedCleanly() == 1);
  CHECK(worker.performedQueriesTotal() == params.queries_per_thread);
}

}  // namespace

int main() {
  testSingleThreadExceptionIsCaughtAndReported();
  testSuccessfulRunStillReturnsTrue();
  testOrdinaryConnectFailureDoesNotTripExceptionFlag();
  testManyConcurrentThreadsAllThrowingDoesNotCrash();
  testFactoryExceptionIsCaught();
  testCreateInfileParserEnablesDollarQuotingOnlyForPgsql();
  testConsecutiveFailuresAbortThread();
  testIntermittentFailuresDoNotAbort();

  std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed"
            << std::endl;

  return g_failures == 0 ? 0 : 1;
}
