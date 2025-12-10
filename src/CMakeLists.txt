SET(BINARY_NAME "pquery${vMajor}")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
SET(COMMON_SOURCES
    cDatabase.cpp
    cDbWorker.cpp
    cInfileParser.cpp
    cIniReader.cpp
    cLogger.cpp
    cPQuery.cpp
    cSqlFileParser.cpp
    main.cpp
    )
#
IF(MYSQL_FOUND)
  SET(MYSQL_SOURCES
      cMysqlDatabase.cpp
      cMysqlWorker.cpp
      cMyGenLogParser.cpp
      cMyBinLogParser.cpp
      )
ENDIF(MYSQL_FOUND)
#
IF(PGSQL_FOUND)
  SET(PGSQL_SOURCES
      cPgsqlDatabase.cpp
      cPgsqlWorker.cpp
      )
ENDIF(PGSQL_FOUND)
#
#
ADD_EXECUTABLE(${BINARY_NAME} ${COMMON_SOURCES}
               ${MYSQL_SOURCES}
               ${PGSQL_SOURCES}
               )
TARGET_LINK_LIBRARIES( ${BINARY_NAME} ${LIBS_TO_LINK} )
#
INSTALL(TARGETS ${BINARY_NAME} DESTINATION bin)
