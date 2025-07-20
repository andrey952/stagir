cmake_minimum_required(VERSION 3.10)

# Добавление статической библиотеки
add_library(logger STATIC src/logger.cpp)

# Задаваем правило для динамического билдинга (при включенной опции BUILD_SHARED_LIBS)
if(BUILD_SHARED_LIBS)
    add_library(logger SHARED src/logger.cpp)
endif()

# Подключаем библиотеку ws2_32.lib (Winsock)
target_link_libraries(logger ws2_32)

# Уточняем правила видимости заголовочных файлов
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Формируем цель установки
install(TARGETS logger DESTINATION lib)
install(FILES include/logger.hpp DESTINATION include)