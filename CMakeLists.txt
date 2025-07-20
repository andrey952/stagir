cmake_minimum_required(VERSION 3.10)
project(logger_project LANGUAGES CXX)

# Установка минимальной версии C++, необходимой для нашего проекта
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем поддержку построения динамических библиотек (если необходимо)
option(BUILD_SHARED_LIBS "Enable building a shared library version" OFF)

# Позволяет включать subdirectories в проект
add_subdirectory(logger_library)
add_subdirectory(stats_collector)

# Устанавливаем переменную пути для удобства установки
set(INSTALL_PATH "${CMAKE_BINARY_DIR}/install")

# Основное приложение (строится отдельно от тестов)
add_executable(my_app stats_collector/stats_collector.cpp)
target_link_libraries(my_app logger)
install(TARGETS my_app DESTINATION bin)

# --- ИНТЕГРАЦИЯ GOOGLE TEST ---

# Добавляем Google Test в проект
add_subdirectory(external/googletest)

# Включаем тестирование
enable_testing()

# Цель для тестов аргументов
add_executable(arg_tests tests/test_args.cpp)
target_link_libraries(arg_tests gtest_main logger)

# Регистрация теста
add_test(NAME arg_tests COMMAND arg_tests)