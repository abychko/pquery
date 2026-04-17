#ifndef __HCOMMON_HPP__
#define __HCOMMON_HPP__

#include <algorithm>
#include <cctype>
#include <string>

#include <eTypes.hpp>

#ifndef PQMAJVERSION
#define PQMAJVERSION "UNKNOWN"
#endif

#ifndef PQVERSION
#define PQVERSION "UNKNOWN"
#endif

#ifndef PQREVISION
#define PQREVISION "UNKNOWN"
#endif

#ifndef PQRELDATE
#define PQRELDATE "UNKNOWN"
#endif

#ifndef PQBUILDDATE
#define PQBUILDDATE __DATE__
#endif

#ifndef MYSQL_FORK
#define MYSQL_FORK "UNKNOWN"
#endif

#if defined(WIN32) || defined(_WIN32)
const std::string FSSEP = "\\";
#else
const std::string FSSEP = "/";
#endif

inline std::string
infiletype_str(eINFILETYPE infiletype) {
  switch (infiletype) {
    case eSQL:
      return "SQL";
    case eGENLOG:
      return "General Log";
    case eBINLOG:
      return "Binary Log";
    default:
      return "UNKNOWN TYPE";
    }
  }


inline std::string
toLowerCase(const std::string& str) {
  std::string lowercased = str;
  std::transform(lowercased.begin(),
    lowercased.end(),
    lowercased.begin(),
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
  );
  return lowercased;
  }


inline std::string
dbtype_str(eDBTYPE type) {
  switch (type) {
    case eMYSQL:
      return "MySQL";
    case ePGSQL:
      return "PostgreSQL";
    default:
      return "UNKNOWN";
    }
  }
#endif
