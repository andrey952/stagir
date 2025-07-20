#include "logger.hpp"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

Logger::Logger(const std::string& filename, LogLevel defaultLevel) : mFilename(filename), mCurrentLevel(defaultLevel) {
    mFileStream.open(filename, std::ios::out | std::ios::app);
}

Logger::~Logger() {
    if (mSockFd != INVALID_SOCKET) {
        closesocket(mSockFd);
        WSACleanup();
    }
    mFileStream.close();
}

void Logger::log(const std::string& message, LogLevel level) {
    if (level >= mCurrentLevel && mFileStream.is_open()) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto timeStr = formatTime();
        mFileStream << "[" << timeStr << "] [" << static_cast<int>(level) << "] " << message << "\n";
        mFileStream.flush();
    }
}

void Logger::setDefaultLogLevel(LogLevel level) {
    if (isValidLevel(level)) {
        mCurrentLevel = level;
    }
}

std::string Logger::formatTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    struct tm* localTm = std::localtime(&t);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTm);
    return std::string(buffer);
}

bool Logger::isValidLevel(LogLevel level) {
    return (level == LogLevel::INFO ||
            level == LogLevel::WARNING ||
            level == LogLevel::CRITICAL);
}

void Logger::init_socket(const std::string& ip_address, uint16_t port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        throw std::runtime_error("Ошибка инициализации WinSock.");
    }

    mSockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mSockFd == INVALID_SOCKET) {
        throw std::runtime_error("Ошибка создания сокета.");
    }

    sockaddr_in serverAddr{};
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip_address.c_str(), &serverAddr.sin_addr);

    if (connect(mSockFd, reinterpret_cast<const sockaddr*>(&serverAddr), sizeof(serverAddr)) != SOCKET_ERROR) {
        // Соединение установлено
    } else {
        throw std::runtime_error("Ошибка соединения с сервером.");
    }
}

void Logger::log_to_socket(const std::string& message, LogLevel level) {
    if (level >= mCurrentLevel && mSockFd != INVALID_SOCKET) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto timeStr = formatTime();
        std::string fullMessage = "[" + timeStr + "] [" + std::to_string(static_cast<int>(level)) + "] " + message + "\n";
        send(mSockFd, fullMessage.c_str(), fullMessage.size(), 0);
    }
}
