#
INCLUDE(FindGit)
# VERSION
FILE(READ "PQuery.version" ver)
STRING(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${ver})
SET(vMajor ${CMAKE_MATCH_1})
STRING(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${ver})
SET(vMinor ${CMAKE_MATCH_1})
STRING(REGEX MATCH "VERSION_PATCH ([0-9]*)" _ ${ver})
SET(vPatch ${CMAKE_MATCH_1})
#
SET(PROJECT_VERSION "${vMajor}.${vMinor}.${vPatch}")
#
# REVISION and last commit date
IF (GIT_FOUND)
  EXECUTE_PROCESS(COMMAND "${GIT_EXECUTABLE}" log -1 --pretty=format:%h
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RESULT_VARIABLE rev OUTPUT_VARIABLE REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
  IF(NOT rev EQUAL 0)
    SET(REVISION "NOTFOUND")
  ENDIF()
  EXECUTE_PROCESS(COMMAND "${GIT_EXECUTABLE}" log -1 --pretty=format:%ad --date=iso
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RESULT_VARIABLE ldate OUTPUT_VARIABLE RELDATE
  OUTPUT_STRIP_TRAILING_WHITESPACE)
  IF(NOT ldate EQUAL 0)
    SET(RELDATE "UNKNOWN")
  ENDIF()
ELSE(GIT_FOUND)
  SET(REVISION "NOTFOUND")
  SET(RELDATE "UNKNOWN")
ENDIF(GIT_FOUND)
#
ADD_DEFINITIONS(-DPQVERSION="${PROJECT_VERSION}")
ADD_DEFINITIONS(-DPQREVISION="${REVISION}")
ADD_DEFINITIONS(-DPQRELDATE="${RELDATE}")
#
MESSAGE(STATUS "PQuery version ${PROJECT_VERSION}, revision ${REVISION} (${RELDATE})")
