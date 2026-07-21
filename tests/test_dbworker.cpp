// test_dbworker.cpp
//
// Lightweight, assert-style unit tests for DbWorker (no external test
// framework, run through CTest). Exercises the worker-thread exception
// handling added to cDbWorker.cpp: an exception thrown inside
// DbWorker::workerThread() (e.g. from Database::connect() or
// createDbInstance()) must be caught, must not terminate the process, must
// be reflected in DbWorker::executeTests()'s return value, and must behave
// correctly under concurrent execution from multiple threads.

#include <cDbWorker.hpp>
#include <cDatabase.hpp>
#include <sWorkerParams.hpp>

#include <atomic>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

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
  enum class Mode { kThrowOnConnect, kFailOnConnect, kSucceed };

  explicit FakeDatabase(Mode mode) : mode_(mode) {}

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
        return true;
    }
    return false;
  }

  std::uint64_t getAffectedRows() override { return 0; }
  bool performRealQuery(const std::string & /*query*/) override {
    return true;
  }
  void processQueryOutput() override {}
  std::uint32_t getWarningsCount() override { return 0; }
  void cleanupResult() override {}

 private:
  Mode mode_;
};

// A minimal concrete DbWorker whose factory/hook methods are configurable
// per test, so we can drive workerThread() through its various branches.
class TestDbWorker : public DbWorker {
 public:
  explicit TestDbWorker(FakeDatabase::Mode mode) : mode_(mode) {}

  std::shared_ptr<Database> createDbInstance() override {
    ++instances_created_;
    if (throw_in_factory_) {
      throw std::runtime_error("simulated factory exception");
    }
    return std::make_shared<FakeDatabase>(mode_);
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
  int threadsFinishedCleanly() const { return threads_finished_cleanly_.load(); }

 private:
  FakeDatabase::Mode mode_;
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
  params.shuffle = true;  // avoid adjustRuntimeParams() forcing threads to 1
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

}  // namespace

int main() {
  testSingleThreadExceptionIsCaughtAndReported();
  testSuccessfulRunStillReturnsTrue();
  testOrdinaryConnectFailureDoesNotTripExceptionFlag();
  testManyConcurrentThreadsAllThrowingDoesNotCrash();
  testFactoryExceptionIsCaught();

  std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed"
            << std::endl;

  return g_failures == 0 ? 0 : 1;
}
