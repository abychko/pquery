#############################################################
#  Find mysqlclient                                         #
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.          #
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.  #
#  MYSQL_FOUND       - True if MySQL found.                 #
#############################################################
#
IF(MYSQL_BASEDIR)
  IF (NOT EXISTS ${MYSQL_BASEDIR})
    MESSAGE(FATAL_ERROR "Directory ${MYSQL_BASEDIR} doesn't exist. Check the path for typos!")
  ENDIF(NOT EXISTS ${MYSQL_BASEDIR})
  MESSAGE(STATUS "MYSQL_BASEDIR is set, looking for ${MYSQL_FORK} in ${MYSQL_BASEDIR}")
ENDIF()
#
IF(MYSQL_FORK STREQUAL MYSQL)
  SET(MYSQL_NAMES mysqlclient)
  ADD_DEFINITIONS(-DMYSQL_FORK="MySQL")
ENDIF()
#
IF(MYSQL_FORK STREQUAL MARIADB)
  SET(MARIADB TRUE)
  SET(MYSQL_NAMES mysqlclient)
  ADD_DEFINITIONS(-DMYSQL_FORK="MariaDB")
ENDIF()
#
IF(MYSQL_FORK STREQUAL PERCONASERVER)
  SET(MYSQL_NAMES perconaserverclient)
  ADD_DEFINITIONS(-DMYSQL_FORK="Percona Server")
ENDIF()
#
IF(MYSQL_FORK STREQUAL PERCONACLUSTER)
  SET(MYSQL_NAMES perconaserverclient mysqlclient)
  ADD_DEFINITIONS(-DMYSQL_FORK="Percona XtraDB Cluster")
ENDIF()
#
IF(MYSQL_FORK STREQUAL WEBSCALESQL)
  SET(MYSQL_NAMES webscalesqlclient webscalesqlclient_r)
  ADD_DEFINITIONS(-DMYSQL_FORK="WebScaleSQL")
ENDIF()
#
IF(MYSQL_INCLUDE_DIR)
  # Already in cache, be silent
  SET(MYSQL_FIND_QUIETLY TRUE)
ENDIF (MYSQL_INCLUDE_DIR)
#
IF(STATIC_MYSQL AND NOT MARIADB)
  SET(_mysql_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
ENDIF(STATIC_MYSQL AND NOT MARIADB)
#
FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
  ${MYSQL_BASEDIR}/include
  ${MYSQL_BASEDIR}/include/mysql
  /usr/local/include/mysql
  /usr/include/mysql
  /usr/local/mysql/include
  )
#
FIND_LIBRARY(MYSQL_LIBRARY
  NAMES ${MYSQL_NAMES}
  IF(MYSQL_BASEDIR)
    PATHS ${MYSQL_BASEDIR}/lib ${MYSQL_BASEDIR}/lib64
    NO_CMAKE_SYSTEM_PATH
  ELSE(MYSQL_BASEDIR)
    PATHS /usr/lib /usr/lib64 /usr/local/lib
          /usr/lib/x86_64-linux-gnu /usr/lib/i386-linux-gnu
          /usr/local/mysql/lib /usr/local/opt/mysql/lib
  ENDIF(MYSQL_BASEDIR)
  PATH_SUFFIXES mysql
  )
#
IF(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  SET(MYSQL_FOUND TRUE)
  SET(MYSQL_LIBRARIES ${MYSQL_LIBRARY} )
ELSE(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  SET(MYSQL_FOUND FALSE)
  SET(MYSQL_LIBRARIES)
ENDIF(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
#
IF(MYSQL_FOUND)
  IF (NOT MYSQL_FIND_QUIETLY)
    MESSAGE(STATUS "Found ${MYSQL_FORK} library: ${MYSQL_LIBRARY}")
    MESSAGE(STATUS "Found ${MYSQL_FORK} includes: ${MYSQL_INCLUDE_DIR}")
  ENDIF (NOT MYSQL_FIND_QUIETLY)
ELSE(MYSQL_FOUND)
  MESSAGE(STATUS "Looked for ${MYSQL_FORK} libraries named ${MYSQL_NAMES}.")
  MESSAGE(FATAL_ERROR "Could NOT find ${MYSQL_FORK} library")
ENDIF(MYSQL_FOUND)
#
MARK_AS_ADVANCED(MYSQL_LIBRARY MYSQL_INCLUDE_DIR)
#
