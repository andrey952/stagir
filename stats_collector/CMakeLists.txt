cmake_minimum_required(VERSION 3.10)

# Проект
add_executable(stats_collector stats_collector.cpp)

# Добавляем необходимую библиотеку Winsock2
target_link_libraries(stats_collector PRIVATE ws2_32)

# Опциональный установочный шаг
install(TARGETS stats_collector DESTINATION bin)