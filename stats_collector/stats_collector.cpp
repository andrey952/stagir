#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <windows.h>       // ws2tcpip.h необходим для сокетов
#include <winsock2.h>      // базовая поддержка сокетов
#include <thread>
#include <chrono>
#include <atomic>

using namespace std;

struct Stats {
    atomic<int> totalMessages{0}, warnings{0}, errors{0}, infos{0};
    atomic<int> minLength{INT_MAX}, maxLength{0}, sumLength{0};
    chrono::steady_clock::time_point lastOutput;
};

Stats global_stats;
const int MAX_BUFFER_SIZE = 1024;

void handle_message(const string& message) {
    istringstream iss(message);
    string token;
    vector<string> tokens;
    while(std::getline(iss, token, ' ')) tokens.push_back(token);

    if(tokens.size() > 2){
        ++global_stats.totalMessages;
        string_view type = tokens[1]; // Пример: [INFO]
        if(type == "[INFO]")
            ++global_stats.infos;
        else if(type == "[WARNING]")
            ++global_stats.warnings;
        else if(type == "[ERROR]")
            ++global_stats.errors;

        int len = tokens.back().size();
        global_stats.sumLength += len;
        global_stats.minLength = min(global_stats.minLength.load(), len);
        global_stats.maxLength = max(global_stats.maxLength.load(), len);
    }
}

void print_stats(int intervalSec, int printAfterNthMsg) {
    while(true){
        this_thread::sleep_for(chrono::seconds(intervalSec));
        if(global_stats.totalMessages >= printAfterNthMsg || global_stats.lastOutput.time_since_epoch().count() == 0){
            cout << "Всего сообщений: " << global_stats.totalMessages << endl;
            cout << "Сообщений INFO: " << global_stats.infos << ", WARNINGS: " << global_stats.warnings << ", ERRORS: " << global_stats.errors << endl;
            cout << "Минимальная длина сообщения: " << global_stats.minLength << endl;
            cout << "Максимальная длина сообщения: " << global_stats.maxLength << endl;
            double avgLen = global_stats.totalMessages ? ((double)global_stats.sumLength / global_stats.totalMessages) : 0;
            cout << fixed << setprecision(2) << "Средняя длина сообщения: " << avgLen << endl;
            global_stats.lastOutput = chrono::steady_clock::now();
        }
    }
}

int main(int argc, char** argv) {
    if(argc < 4){
        cerr << "Использование: " << argv[0] << " <IP> <Порт> <интервал_секунд> <после_количества_сообщений>\n";
        return EXIT_FAILURE;
    }

    string ipAddress = argv[1];
    unsigned short port = atoi(argv[2]);
    int intervalSeconds = atoi(argv[3]), printAfterNthMsg = atoi(argv[4]);

    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR){
        cerr << "Ошибка инициализации WinSock.\n";
        return EXIT_FAILURE;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == INVALID_SOCKET){
        cerr << "Ошибка создания сокета.\n";
        return EXIT_FAILURE;
    }

    sockaddr_in servaddr{};
    ZeroMemory(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &servaddr.sin_addr);

    if(connect(sockfd, reinterpret_cast<const sockaddr*>(&servaddr), sizeof(servaddr)) != SOCKET_ERROR){
        thread statPrinter(print_stats, intervalSeconds, printAfterNthMsg);

        char buffer[MAX_BUFFER_SIZE];
        while(true){
            int bytesRead = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
            if(bytesRead <= 0) break;
            string message(buffer, bytesRead);
            handle_message(message);
        }

        statPrinter.join();
        closesocket(sockfd); // Закрытие сокета в Windows
        WSACleanup();        // Завершаем использование WinSock
    } else {
        cerr << "Ошибка соединения с сервером.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}