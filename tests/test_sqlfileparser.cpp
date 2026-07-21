// test_sqlfileparser.cpp
//
// Lightweight, assert-style unit tests for SqlFileParser (no external test
// framework, run through CTest). Each test writes a small temporary .sql
// file, runs the parser over it, and checks the resulting statement list.

#include <cSqlFileParser.hpp>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
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

// Writes `content` to a uniquely-named temporary file and returns its path.
// The caller is responsible for removing the file (removeFile()).
std::string writeTempFile(const std::string &name, const std::string &content) {
  std::string path = "sqlfileparser_" + name + ".tmp.sql";
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << content;
  out.close();
  return path;
}

void removeFile(const std::string &path) { std::remove(path.c_str()); }

// Runs the parser over `content` (via a temp file) and returns the parsed
// statement list. Asserts that parsing itself succeeded.
std::vector<std::string> parse(const std::string &name,
                               const std::string &content,
                               bool dollarQuoting = false) {
  std::string path = writeTempFile(name, content);

  SqlFileParser parser(dollarQuoting);
  auto queryList = std::make_shared<std::vector<std::string>>();
  bool ok = parser.loadQueriesFromFile(queryList, path);
  removeFile(path);

  CHECK(ok);
  return *queryList;
}

void testBasicSplit() {
  auto queries = parse("basic", "SELECT 1;\nSELECT 2;\n");
  CHECK(queries.size() == 2);
  if (queries.size() == 2) {
    CHECK(queries[0] == "SELECT 1");
    CHECK(queries[1] == "SELECT 2");
  }
}

void testCustomDelimiterSwitchesBackToSemicolon() {
  std::string sql =
      "DELIMITER $$\n"
      "CREATE PROCEDURE test()\n"
      "BEGIN\n"
      "  SELECT 1;\n"
      "END$$\n"
      "DELIMITER ;\n"
      "SELECT 2;\n";

  auto queries = parse("delimiter", sql);
  CHECK(queries.size() == 2);
  if (queries.size() == 2) {
    // The procedure body's internal ';' must not have split the statement.
    CHECK(queries[0].find("SELECT 1;") != std::string::npos);
    CHECK(queries[0].find("CREATE PROCEDURE test()") != std::string::npos);
    CHECK(queries[0].find("END") != std::string::npos);
    CHECK(queries[1] == "SELECT 2");
  }
}

void testSemicolonInsideSingleQuotesIsNotASplit() {
  auto queries =
      parse("singlequote", "INSERT INTO t VALUES ('a;b');\nSELECT 3;\n");
  CHECK(queries.size() == 2);
  if (queries.size() == 2) {
    CHECK(queries[0] == "INSERT INTO t VALUES ('a;b')");
    CHECK(queries[1] == "SELECT 3");
  }
}

void testSemicolonInsideDoubleQuotesIsNotASplit() {
  auto queries = parse("doublequote", "INSERT INTO t VALUES (\"a;b\");\n");
  CHECK(queries.size() == 1);
  if (queries.size() == 1) {
    CHECK(queries[0] == "INSERT INTO t VALUES (\"a;b\")");
  }
}

void testSemicolonInsideBackticksIsNotASplit() {
  auto queries = parse("backtick", "SELECT * FROM `my;table`;\n");
  CHECK(queries.size() == 1);
  if (queries.size() == 1) {
    CHECK(queries[0] == "SELECT * FROM `my;table`");
  }
}

void testEscapedQuoteInsideStringIsNotClosingIt() {
  // The escaped quote (\') must not be treated as the end of the string, so
  // the ';' right after "test" is still inside quotes and must not split.
  auto queries =
      parse("escaped", "INSERT INTO t VALUES ('it\\'s a test;done');\n");
  CHECK(queries.size() == 1);
}

void testSingleLineDashCommentIsSkipped() {
  auto queries = parse("dashcomment", "-- this is a comment\nSELECT 1;\n");
  CHECK(queries.size() == 1);
  if (queries.size() == 1) {
    CHECK(queries[0] == "SELECT 1");
  }
}

void testSingleLineHashCommentIsSkipped() {
  auto queries =
      parse("hashcomment", "# this is a comment\nSELECT 1;\n# trailing\n");
  CHECK(queries.size() == 1);
  if (queries.size() == 1) {
    CHECK(queries[0] == "SELECT 1");
  }
}

void testBlockCommentSpanningMultipleLinesIsSkipped() {
  auto queries =
      parse("blockcomment",
            "/* this is a\nmulti-line comment */\nSELECT 1;\n/* trailing */\n");
  CHECK(queries.size() == 1);
  if (queries.size() == 1) {
    CHECK(queries[0] == "SELECT 1");
  }
}

void testEmptyFileProducesNoQueries() {
  auto queries = parse("empty", "");
  CHECK(queries.empty());
}

void testFileWithOnlyCommentsProducesNoQueries() {
  auto queries = parse("onlycomments",
                       "-- nothing here\n# still nothing\n/* nor here */\n");
  CHECK(queries.empty());
}

void testMissingFileFailsGracefully() {
  SqlFileParser parser;
  auto queryList = std::make_shared<std::vector<std::string>>();
  bool ok =
      parser.loadQueriesFromFile(queryList, "/no/such/file/does-not-exist.sql");
  CHECK(!ok);
  CHECK(queryList->empty());
}

void testNullQueryListIsRejected() {
  SqlFileParser parser;
  std::string path = writeTempFile("nullcheck", "SELECT 1;\n");
  bool ok = parser.loadQueriesFromFile(nullptr, path);
  removeFile(path);
  CHECK(!ok);
}

void testDollarQuoteFunctionBodyIsOneStatement() {
  auto queries = parse(
      "dollarfunc",
      "CREATE FUNCTION f() RETURNS int AS $$ BEGIN SELECT 1; SELECT 2; END "
      "$$ LANGUAGE plpgsql;\n"
      "SELECT 3;\n",
      /*dollarQuoting=*/true);
  CHECK(queries.size() == 2);
  if (queries.size() == 2) {
    CHECK(queries[0].find("SELECT 1;") != std::string::npos);
    CHECK(queries[1] == "SELECT 3");
  }
}

void testDollarQuoteWithNamedTagFunctionBodyIsOneStatement() {
  auto queries = parse(
      "dollarfuncnamed",
      "CREATE FUNCTION f() RETURNS int AS $body$ BEGIN SELECT 1; SELECT 2; "
      "END $body$ LANGUAGE plpgsql;\n"
      "SELECT 3;\n",
      /*dollarQuoting=*/true);
  CHECK(queries.size() == 2);
  if (queries.size() == 2) {
    CHECK(queries[0].find("SELECT 1;") != std::string::npos);
    CHECK(queries[1] == "SELECT 3");
  }
}

void testDollarQuoteWithUnescapedSingleQuoteInside() {
  auto queries =
      parse("dollarquoteapostrophe", "SELECT $$it's a 'test'; done$$;\n",
            /*dollarQuoting=*/true);
  CHECK(queries.size() == 1);
}

void testTwoConsecutiveDollarQuoteBlocks() {
  auto queries = parse("dollartwo",
                       "SELECT $$one; two$$;\nSELECT $tag$three; four$tag$;\n",
                       /*dollarQuoting=*/true);
  CHECK(queries.size() == 2);
}

void testDollarNumberedPlaceholdersAreNotDollarQuotes() {
  auto queries =
      parse("dollarplaceholders",
            "PREPARE p AS SELECT * FROM t WHERE a = $1 AND b = $2;\n",
            /*dollarQuoting=*/true);
  CHECK(queries.size() == 1);
  if (queries.size() == 1) {
    CHECK(queries[0] == "PREPARE p AS SELECT * FROM t WHERE a = $1 AND b = $2");
  }
}

void testDollarQuoteInnerTagIsNotRealNesting() {
  auto queries =
      parse("dollarnested",
            "SELECT $outer$ text $inner$more$inner$ ; text $outer$;\n",
            /*dollarQuoting=*/true);
  CHECK(queries.size() == 1);
}

void testDollarSignInIdentifierWithoutDollarQuotingEnabled() {
  auto queries = parse("dollarmysql", "SELECT a$b$c;\nSELECT 2;\n",
                       /*dollarQuoting=*/false);
  CHECK(queries.size() == 2);
}

void testUnterminatedDollarQuoteDoesNotCrash() {
  auto queries = parse("dollarunterminated", "SELECT $$ text without an end\n",
                       /*dollarQuoting=*/true);
  CHECK(queries.size() == 1);
}

}  // namespace

int main() {
  testBasicSplit();
  testCustomDelimiterSwitchesBackToSemicolon();
  testSemicolonInsideSingleQuotesIsNotASplit();
  testSemicolonInsideDoubleQuotesIsNotASplit();
  testSemicolonInsideBackticksIsNotASplit();
  testEscapedQuoteInsideStringIsNotClosingIt();
  testSingleLineDashCommentIsSkipped();
  testSingleLineHashCommentIsSkipped();
  testBlockCommentSpanningMultipleLinesIsSkipped();
  testEmptyFileProducesNoQueries();
  testFileWithOnlyCommentsProducesNoQueries();
  testMissingFileFailsGracefully();
  testNullQueryListIsRejected();
  testDollarQuoteFunctionBodyIsOneStatement();
  testDollarQuoteWithNamedTagFunctionBodyIsOneStatement();
  testDollarQuoteWithUnescapedSingleQuoteInside();
  testTwoConsecutiveDollarQuoteBlocks();
  testDollarNumberedPlaceholdersAreNotDollarQuotes();
  testDollarQuoteInnerTagIsNotRealNesting();
  testDollarSignInIdentifierWithoutDollarQuotingEnabled();
  testUnterminatedDollarQuoteDoesNotCrash();

  std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed"
            << std::endl;

  return g_failures == 0 ? 0 : 1;
}
