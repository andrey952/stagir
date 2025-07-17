cmake_minimum_required(VERSION 3.10)

# Проверяем наличие библиотек WinSock2
find_package(WINSOCK2 REQUIRED)

# Создание исполнимого файла
add_executable(stats_collector stats_collector.cpp)

# Связывание с библиотекой WINSOCK2
target_link_libraries(stats_collector PRIVATE WINSOCK2::WINSOCK2)

# Установочная цель
install(TARGETS stats_collector DESTINATION bin)