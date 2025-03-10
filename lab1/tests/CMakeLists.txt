# Исполняемый файл для тестов
add_executable(Tests tests.cpp)

# Подключаем библиотеку Google Test
target_link_libraries(Tests PRIVATE GTest::gtest GTest::gtest_main)

# Добавляем тест в CTest
add_test(NAME EmployeeTests COMMAND Tests)