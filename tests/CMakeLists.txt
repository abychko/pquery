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
