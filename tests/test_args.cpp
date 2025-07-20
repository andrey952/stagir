#include <gtest/gtest.h>
#include "../stats_collector/stats_collector.cpp"

// Вспомогательная функция для вызова main() с фиктивными аргументами
int runMainWithArgs(int argc, char** argv) {
    return main(argc, argv);
}

// Тест 1: Проверка корректных аргументов
TEST(ArgTests, ValidArguments) {
    const char* args[] = {"my_app", "127.0.0.1", "5000", "10", "100"};
    int result = runMainWithArgs(sizeof(args)/sizeof(const char*) - 1, const_cast<char**>(args));
    EXPECT_EQ(result, 0);  // Успех приложения возвращает 0
}

// Тест 2: Недостаток аргументов
TEST(ArgTests, MissingArguments) {
    const char* args[] = {"my_app", "127.0.0.1", "5000"};  // Пропущен аргумент
    int result = runMainWithArgs(sizeof(args)/sizeof(const char*) - 1, const_cast<char**>(args));
    EXPECT_NE(result, 0);  // Ожидается ненулевое значение при ошибке
}

// Тест 3: Избыточные аргументы
TEST(ArgTests, ExcessArguments) {
    const char* args[] = {"my_app", "127.0.0.1", "5000", "10", "100", "extra_arg"};
    int result = runMainWithArgs(sizeof(args)/sizeof(const char*) - 1, const_cast<char**>(args));
    EXPECT_NE(result, 0);  // Ожидается, что приложение отклонит лишние аргументы
}

// Тест 4: Некорректные аргументы (например, неправильный порт)
TEST(ArgTests, InvalidArguments) {
    const char* args[] = {"my_app", "127.0.0.1", "-1", "10", "100"};  // Негативный порт
    int result = runMainWithArgs(sizeof(args)/sizeof(const char*) - 1, const_cast<char**>(args));
    EXPECT_NE(result, 0);  // Ожидается отказ от неправильного аргумента
}

// Тест 5: Пустые аргументы
TEST(ArgTests, EmptyArguments) {
    const char* args[] = {"my_app", "", "5000", "10", "100"};  // Пустой IP-адрес
    int result = runMainWithArgs(sizeof(args)/sizeof(const char*) - 1, const_cast<char**>(args));
    EXPECT_NE(result, 0);  // Ожидается отзыв на пустой аргумент
}