#include <iostream>
#include <sstream>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>
#include <cstdio>
#include <thread>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

struct Stats {
    std::atomic<int> totalMessages{0}, warnings{0}, errors{0}, infos{0};
    std::atomic<int> minLength{INT_MAX}, maxLength{0}, sumLength{0};
    std::chrono::steady_clock::time_point lastOutput;
};

Stats global_stats;
const int MAX_BUFFER_SIZE = 1024;

void handle_message(const std::string& message) {
    std::istringstream iss(message);
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(iss, token, ' ')) {
        tokens.push_back(token);
    }

    if (tokens.size() > 2) {
        ++global_stats.totalMessages;
        std::string_view type = tokens[1]; // Пример: [INFO]
        if (type == "[INFO]") ++global_stats.infos;
        else if (type == "[WARNING]") ++global_stats.warnings;
        else if (type == "[ERROR]") ++global_stats.errors;

        int len = tokens.back().length();
        global_stats.sumLength += len;
        global_stats.minLength = std::min(global_stats.minLength.load(), len);
        global_stats.maxLength = std::max(global_stats.maxLength.load(), len);
    }
}

void print_stats(int intervalSec, int printAfterNthMsg) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(intervalSec));
        if (global_stats.totalMessages >= printAfterNthMsg || global_stats.lastOutput.time_since_epoch().count() == 0) {
            std::cout << "Всего сообщений: " << global_stats.totalMessages << std::endl;
            std::cout << "Сообщений INFO: " << global_stats.infos << ", WARNINGS: " << global_stats.warnings << ", ERRORS: " << global_stats.errors << std::endl;
            std::cout << "Минимальная длина сообщения: " << global_stats.minLength << std::endl;
            std::cout << "Максимальная длина сообщения: " << global_stats.maxLength << std::endl;
            double avgLen = global_stats.totalMessages ? ((double)global_stats.sumLength / global_stats.totalMessages) : 0;
            std::cout << "Средняя длина сообщения: " << avgLen << std::endl;
            global_stats.lastOutput = std::chrono::steady_clock::now();
        }
    }
}

// Извлекаем логику из main() в отдельную функцию
void start_application(const std::string& ipAddress, unsigned short port, int intervalSeconds, int printAfterNthMsg) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        std::cerr << "Ошибка инициализации WinSock.\n";
        return;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета.\n";
        return;
    }

    sockaddr_in servaddr{};
    ZeroMemory(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &servaddr.sin_addr);

    if (connect(sockfd, reinterpret_cast<const sockaddr*>(&servaddr), sizeof(servaddr)) != SOCKET_ERROR) {
        std::thread statPrinter(print_stats, intervalSeconds, printAfterNthMsg);

        char buffer[MAX_BUFFER_SIZE];
        while (true) {
            int bytesRead = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
            if (bytesRead <= 0) break;
            std::string message(buffer, bytesRead);
            handle_message(message);
        }

        statPrinter.join();
        closesocket(sockfd);
        WSACleanup();
    } else {
        std::cerr << "Ошибка соединения с сервером.\n";
    }
}

// Функция main() теперь просто делегирует вызов в отдельную функцию
int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Использование: " << argv[0] << " <IP> <Порт> <Интервал в секундах> <Количество сообщений>\n";
        return EXIT_FAILURE;
    }

    std::string ipAddress = argv[1];
    unsigned short port = std::stoi(argv[2]);
    int intervalSeconds = std::stoi(argv[3]);
    int printAfterNthMsg = std::stoi(argv[4]);

    start_application(ipAddress, port, intervalSeconds, printAfterNthMsg);
    return 0;
}