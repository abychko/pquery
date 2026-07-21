#ifndef HCOMMON_HPP
#define HCOMMON_HPP

#include <algorithm>
#include <cctype>
#include <cstdint>
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

inline constexpr char FSSEP = '/';

// Consecutive query failures after which a worker thread aborts its loop.
inline constexpr std::uint16_t MAX_CON_FAILURES = 250;

inline std::string infiletype_str(eINFILETYPE infiletype) {
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

inline std::string toLowerCase(const std::string &str) {
  std::string lowercased = str;
  std::transform(
      lowercased.begin(), lowercased.end(), lowercased.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return lowercased;
}

inline std::string dbtype_str(eDBTYPE type) {
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
