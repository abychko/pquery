#include <cLogger.hpp>
#include <hCommon.hpp>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

Logger::Logger() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::ios_base::sync_with_stdio(false);
}

Logger::~Logger() {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  if (logFile.is_open()) {
    logFile.close();
  }
}

void Logger::addSeparation(char what, int lenght) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::lock_guard<std::mutex> lock(logMutex);
  std::ios_base::fmtflags f(logFile.flags());
  logFile << std::setfill(what) << std::setw(lenght) << "\n";
  logFile.flags(f);
}

void Logger::setLogFilePath(std::string filePath) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::lock_guard<std::mutex> lock(logMutex);
  pendingLogFilePath = filePath;
}

bool Logger::initLogFile(std::string filePath) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::lock_guard<std::mutex> lock(logMutex);
  // a path set via setLogFilePath() (e.g. from -L/--master-logfile) takes
  // precedence over the path passed in, but only for the next file opened
  std::string targetPath = filePath;
  if (!pendingLogFilePath.empty()) {
    targetPath = pendingLogFilePath;
    pendingLogFilePath.clear();
  }

  if (logFile.is_open()) {
    logFile.close();
    if (logFile.is_open()) {
      std::cerr << "Can't open log file: " << std::string(strerror(errno))
                << std::endl;
      return false;
    }
  }
  logFile.open(targetPath, std::ios::trunc);
  if (!logFile.is_open()) {
    std::cerr << "Can't open log file: " << std::string(strerror(errno))
              << std::endl;
    return false;
  }
  return true;
}

void Logger::setPrecision(int precision) {
  std::lock_guard<std::mutex> lock(logMutex);
  logFile.precision(precision);
  logFile << std::fixed;
}

void Logger::flushLog() {
  std::lock_guard<std::mutex> lock(logMutex);
  if (!logFile.is_open()) {
    std::cerr << "Log file is not open, can't flush()" << std::endl;
    return;
  }
  logFile.flush();
  if (logFile.fail()) {
    throw std::runtime_error("Can't flush() log file: " +
                             std::string(strerror(errno)));
  }
}

void Logger::addRecordToLog(std::string message) {
  std::lock_guard<std::mutex> lock(logMutex);
  logFile << message << "\n";
  if (logFile.fail()) {
    throw std::runtime_error("Can't write to log file: " +
                             std::string(strerror(errno)));
  }
}

void Logger::addPartialRecord(std::string message) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif
  std::lock_guard<std::mutex> lock(logMutex);
  logFile << message;
  if (logFile.fail()) {
    throw std::runtime_error("Can't write to log file: " +
                             std::string(strerror(errno)));
  }
}
