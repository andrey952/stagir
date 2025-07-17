#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>
#include <sys/types.h> // For AF_INET etc.
#include <sys/socket.h> // For socket(), connect(), send() and recv()
#include <arpa/inet.h> // For sockaddr_in and inet_addr()
#include <unistd.h>     // For close()

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static void init(const std::string& filename, const LogLevel default_level);
    static void setDefaultLogLevel(LogLevel level);
    static void log(const std::string& message, LogLevel level = LogLevel::INFO);
private:
    static std::ofstream fileStream;
    static std::mutex mutex_;
    static LogLevel currentLevel;
    
    static bool isValidLevel(LogLevel level);
    static std::string formatTime();
};

#endif /* LOGGER_HPP */