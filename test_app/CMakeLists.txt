cmake_minimum_required(VERSION 3.10)

# Ищем нашу собственную библиотеку logger
find_package(logger REQUIRED PATHS ${PROJECT_SOURCE_DIR}/../logger_library)

# Добавляем исполняемый файл
add_executable(test_app main.cpp)

# Привязываем зависимости нашей библиотеки
target_link_libraries(test_app PRIVATE logger)

# Установка приложения
install(TARGETS test_app DESTINATION bin)