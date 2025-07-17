cmake_minimum_required(VERSION 3.10)
project(logger_project LANGUAGES CXX)

# Установка минимальной версии C++, необходимой для нашего проекта
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем поддержку построения динамических библиотек (если необходимо)
option(BUILD_SHARED_LIBS "Enable building a shared library version" OFF)

# Позволяет включать subdirectories в проект
add_subdirectory(logger_library)
add_subdirectory(test_app)
add_subdirectory(stats_collector)

# Устанавливаем переменную пути для удобства установки
set(INSTALL_PATH "${CMAKE_BINARY_DIR}/install")

# Конкретизируем цели установки
install(DIRECTORY logger_library/include DESTINATION include)
install(DIRECTORY test_app DESTINATION bin)
install(DIRECTORY stats_collector DESTINATION bin)