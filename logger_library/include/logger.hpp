#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdio>

// Основные заголовки для работы с сокетами (помещаем в начало)
#include <winsock2.h>     // Основной заголовочный файл для работы с сокетами
#include <windows.h>      // Стандартная библиотека Windows
#include <ws2tcpip.h>     // Заголовочный файл для TCP/IP операций

enum class LogLevel {
    INFO,
    WARNING,
    CRITICAL
};

class Logger {
public:
    Logger(const std::string& filename, LogLevel defaultLevel = LogLevel::INFO);
    ~Logger();

    void log(const std::string& message, LogLevel level = LogLevel::INFO);
    void setDefaultLogLevel(LogLevel level);

    // Дополнительная опция для сокетного логирования
    void init_socket(const std::string& ip_address, uint16_t port);
    void log_to_socket(const std::string& message, LogLevel level = LogLevel::INFO);

private:
    std::string mFilename;
    std::ofstream mFileStream;
    std::mutex mMutex;
    LogLevel mCurrentLevel;
    SOCKET mSockFd = INVALID_SOCKET; // Объект сокета и его инициализация
    std::string formatTime();
    bool isValidLevel(LogLevel level);
};

#endif // LOGGER_HPP
