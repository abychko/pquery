// test_inireader.cpp
//
// Lightweight, assert-style unit tests for INIReader (no external test
// framework, run through CTest).

#include <cIniReader.hpp>
#include <eTypes.hpp>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
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

std::string writeTempFile(const std::string &name, const std::string &content) {
  std::string path = "inireader_" + name + ".tmp.cfg";
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << content;
  out.close();
  return path;
}

void removeFile(const std::string &path) { std::remove(path.c_str()); }

void testSectionsAndDefaultValues() {
  std::string path = writeTempFile("sections",
                                   "[master]\n"
                                   "logdir = /tmp\n"
                                   "\n"
                                   "[node0]\n"
                                   "database = test\n");

  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);

  auto sections = reader.GetSections();
  CHECK(sections.size() == 2);
  if (sections.size() == 2) {
    CHECK(sections[0] == "master");
    CHECK(sections[1] == "node0");
  }

  CHECK(reader.Get("master", "logdir", "default") == "/tmp");
  CHECK(reader.Get("node0", "database", "default") == "test");
  // missing key -> default value
  CHECK(reader.Get("node0", "nosuchkey", "fallback") == "fallback");
  // missing section -> default value
  CHECK(reader.Get("nosuchsection", "database", "fallback") == "fallback");
}

void testGetIntegerPlainValues() {
  std::string path =
      writeTempFile("plainint", "[s]\nthreads = 10\nqueries = 10000\n");
  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);
  CHECK(reader.GetInteger("s", "threads", -1) == 10);
  CHECK(reader.GetInteger("s", "queries", -1) == 10000);
  // missing key falls back to the supplied default
  CHECK(reader.GetInteger("s", "missing", 42) == 42);
}

void testGetIntegerSuffixes() {
  std::string path = writeTempFile("suffixes",
                                   "[s]\n"
                                   "k_val = 2K\n"
                                   "m_val = 3M\n"
                                   "g_val = 1G\n"
                                   "t_val = 1T\n");
  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);
  CHECK(reader.GetInteger("s", "k_val", -1) == 2LL * 1024);
  CHECK(reader.GetInteger("s", "m_val", -1) == 3LL * 1024 * 1024);
  CHECK(reader.GetInteger("s", "g_val", -1) == 1LL * 1024 * 1024 * 1024);
  CHECK(reader.GetInteger("s", "t_val", -1) == 1LL * 1024 * 1024 * 1024 * 1024);
}

void testGetIntegerOverflowGuard() {
  std::string path =
      writeTempFile("overflow", "[s]\nbig = 9223372036854775807T\n");
  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);

  bool threw = false;
  try {
    reader.GetInteger("s", "big", 0);
  } catch (const std::overflow_error &) {
    threw = true;
  }
  CHECK(threw);
}

void testGetIntegerInvalidSuffixThrows() {
  std::string path = writeTempFile("badsuffix", "[s]\nval = 10X\n");
  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);

  bool threw = false;
  try {
    reader.GetInteger("s", "val", 0);
  } catch (const std::invalid_argument &) {
    threw = true;
  }
  CHECK(threw);
}

void testGetBoolean() {
  std::string path = writeTempFile("bools",
                                   "[s]\n"
                                   "b_true = true\n"
                                   "b_yes = Yes\n"
                                   "b_on = ON\n"
                                   "b_one = 1\n"
                                   "b_false = false\n"
                                   "b_no = No\n"
                                   "b_off = OFF\n"
                                   "b_zero = 0\n");
  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);
  CHECK(reader.GetBoolean("s", "b_true", false) == true);
  CHECK(reader.GetBoolean("s", "b_yes", false) == true);
  CHECK(reader.GetBoolean("s", "b_on", false) == true);
  CHECK(reader.GetBoolean("s", "b_one", false) == true);
  CHECK(reader.GetBoolean("s", "b_false", true) == false);
  CHECK(reader.GetBoolean("s", "b_no", true) == false);
  CHECK(reader.GetBoolean("s", "b_off", true) == false);
  CHECK(reader.GetBoolean("s", "b_zero", true) == false);
  // missing key -> default
  CHECK(reader.GetBoolean("s", "missing", true) == true);
  CHECK(reader.GetBoolean("s", "missing", false) == false);
}

void testMissingFileReturnsNegativeOneParseError() {
  INIReader reader("/no/such/file/does-not-exist.cfg");
  CHECK(reader.ParseError() == -1);
}

void testMalformedLineSetsPositiveParseError() {
  std::string path = writeTempFile("malformed",
                                   "[s]\n"
                                   "good = 1\n"
                                   "this is not a valid line at all\n"
                                   "also_good = 2\n");
  INIReader reader(path);
  removeFile(path);

  // malformed line is line 3 (1-indexed)
  CHECK(reader.ParseError() == 3);
  CHECK(reader.ParseError() > 0);
  // parsing continues after the malformed line
  CHECK(reader.Get("s", "good", "") == "1");
  CHECK(reader.Get("s", "also_good", "") == "2");
}

void testWhitespaceOnlyLineIsNotAnError() {
  // Regression test: a line containing only whitespace must not be treated
  // as a malformed line.
  std::string path = writeTempFile("whitespace",
                                   "[s]\n"
                                   "key1 = value1\n"
                                   "   \n"
                                   "\t\n"
                                   "key2 = value2\n");
  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);
  CHECK(reader.Get("s", "key1", "") == "value1");
  CHECK(reader.Get("s", "key2", "") == "value2");
}

void testGetDbType() {
  std::string path = writeTempFile("dbtype",
                                   "[s1]\ndbtype = mysql\n"
                                   "[s2]\ndbtype = MariaDB\n"
                                   "[s3]\ndbtype = postgres\n"
                                   "[s4]\ndbtype = PostgreSQL\n");
  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);
  CHECK(reader.getDbType("s1", "dbtype", eNONE) == eMYSQL);
  CHECK(reader.getDbType("s2", "dbtype", eNONE) == eMYSQL);
  CHECK(reader.getDbType("s3", "dbtype", eNONE) == ePGSQL);
  CHECK(reader.getDbType("s4", "dbtype", eNONE) == ePGSQL);
  // missing key -> default
  CHECK(reader.getDbType("s1", "missing", eNONE) == eNONE);
}

void testGetInfileType() {
  std::string path = writeTempFile("infiletype",
                                   "[s1]\ninfiletype = sql\n"
                                   "[s2]\ninfiletype = genlog\n"
                                   "[s3]\ninfiletype = binlog\n"
                                   "[s4]\ninfiletype = bogus\n");
  INIReader reader(path);
  removeFile(path);

  CHECK(reader.ParseError() == 0);
  CHECK(reader.getInfileType("s1", "infiletype", eUNKNOWN) == eSQL);
  CHECK(reader.getInfileType("s2", "infiletype", eUNKNOWN) == eGENLOG);
  CHECK(reader.getInfileType("s3", "infiletype", eUNKNOWN) == eBINLOG);
  CHECK(reader.getInfileType("s4", "infiletype", eUNKNOWN) == eUNKNOWN);
  // missing key -> default
  CHECK(reader.getInfileType("s1", "missing", eSQL) == eSQL);
}

}  // namespace

int main() {
  testSectionsAndDefaultValues();
  testGetIntegerPlainValues();
  testGetIntegerSuffixes();
  testGetIntegerOverflowGuard();
  testGetIntegerInvalidSuffixThrows();
  testGetBoolean();
  testMissingFileReturnsNegativeOneParseError();
  testMalformedLineSetsPositiveParseError();
  testWhitespaceOnlyLineIsNotAnError();
  testGetDbType();
  testGetInfileType();

  std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed"
            << std::endl;

  return g_failures == 0 ? 0 : 1;
}
