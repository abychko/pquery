#
# Unit tests (no external test framework, no live database required).
# Each test binary re-compiles the relevant translation unit(s) from src/
# directly, rather than linking against the pquery3 target.
#
SET(PQUERY_SRC_DIR "${CMAKE_SOURCE_DIR}/src")
INCLUDE_DIRECTORIES(${PQUERY_SRC_DIR})
#
ADD_EXECUTABLE(test_sqlfileparser
    test_sqlfileparser.cpp
    ${PQUERY_SRC_DIR}/cSqlFileParser.cpp
    ${PQUERY_SRC_DIR}/cInfileParser.cpp
    )
ADD_TEST(NAME SqlFileParserTests COMMAND test_sqlfileparser)
#
ADD_EXECUTABLE(test_inireader
    test_inireader.cpp
    ${PQUERY_SRC_DIR}/cIniReader.cpp
    )
ADD_TEST(NAME IniReaderTests COMMAND test_inireader)
#
FIND_PACKAGE(Threads REQUIRED)
ADD_EXECUTABLE(test_dbworker
    test_dbworker.cpp
    ${PQUERY_SRC_DIR}/cDbWorker.cpp
    ${PQUERY_SRC_DIR}/cDatabase.cpp
    ${PQUERY_SRC_DIR}/cLogger.cpp
    ${PQUERY_SRC_DIR}/cSqlFileParser.cpp
    ${PQUERY_SRC_DIR}/cInfileParser.cpp
    )
TARGET_LINK_LIBRARIES(test_dbworker Threads::Threads)
ADD_TEST(NAME DbWorkerTests COMMAND test_dbworker)
#
ADD_EXECUTABLE(test_workerconfig
    test_workerconfig.cpp
    ${PQUERY_SRC_DIR}/cWorkerConfig.cpp
    ${PQUERY_SRC_DIR}/cIniReader.cpp
    ${PQUERY_SRC_DIR}/cLogger.cpp
    )
TARGET_COMPILE_DEFINITIONS(test_workerconfig PRIVATE HAVE_MYSQL HAVE_PGSQL)
ADD_TEST(NAME WorkerConfigTests COMMAND test_workerconfig)
#
