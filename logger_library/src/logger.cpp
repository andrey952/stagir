#include "logger.hpp"
#include <windows.h>      // winsock2.h должен идти первым
#include <winsock2.h>     // для работы с сокетами в Windows
#include <ws2tcpip.h>     // дополнительные TCP/IP функции
#include <iostream>
#pragma comment(lib,"Ws2_32.lib") // подключаем библиотеку WS2_32 для сокетов

SOCKET sockfd = INVALID_SOCKET;

std::ofstream Logger::fileStream;
std::mutex Logger::mutex_;
LogLevel Logger::currentLevel = LogLevel::INFO;

void Logger::init(const std::string &filename, const LogLevel default_level) {
    if (!isValidLevel(default_level)) return;
    currentLevel = default_level;
    fileStream.open(filename, std::ios_base::app); // Открываем файл для дописи
}

bool Logger::isValidLevel(LogLevel level) {
    switch(level) {
        case LogLevel::INFO:
        case LogLevel::WARNING:
        case LogLevel::ERROR:
            return true;
        default:
            return false;
    }
}

void Logger::setDefaultLogLevel(LogLevel level) {
    if(isValidLevel(level))
        currentLevel = level;
}

void Logger::log(const std::string &message, LogLevel level) {
    if (level >= currentLevel && fileStream.is_open()) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto timeStr = formatTime();
        fileStream << "[" << timeStr << "] [" << to_string(level) << "] " << message << "\n";
        fileStream.flush(); // Немедленное сохранение изменений
    }
}

std::string Logger::formatTime() {
    char buffer[80];
    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    sprintf_s(buffer, sizeof(buffer),
              "%04u-%02u-%02u %02u:%02u:%02u",
              sysTime.wYear, sysTime.wMonth, sysTime.wDay,
              sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
    return std::string(buffer);
}

// Сокеты для отправки логов удалённому серверу
void Logger::init_socket(const std::string &ip_address, uint16_t port, const LogLevel default_level) {
    if (!isValidLevel(default_level)) return;
    currentLevel = default_level;

    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR) {
        throw std::runtime_error("Ошибка инициализации WinSock.");
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        throw std::runtime_error("Ошибка создания сокета.");
    }

    sockaddr_in serverAddr{};
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip_address.c_str(), &serverAddr.sin_addr);

    if(connect(sockfd, reinterpret_cast<const sockaddr*>(&serverAddr), sizeof(serverAddr)) != SOCKET_ERROR) {
        // Успешное соединение, используем этот сокет для дальнейшего логирования
    } else {
        throw std::runtime_error("Ошибка соединения с сервером.");
    }
}

void Logger::log_to_socket(const std::string &message, LogLevel level) {
    if (level >= currentLevel && sockfd != INVALID_SOCKET) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto timeStr = formatTime();
        std::string fullMessage = "[" + timeStr + "] [" + to_string(level) + "] " + message + "\n";
        send(sockfd, fullMessage.c_str(), fullMessage.size(), 0);
    }
}