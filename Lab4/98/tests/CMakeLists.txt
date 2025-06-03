# Добавляем исполняемый файл для тестов
add_executable(ipc_tests ipc_tests.cpp)

# Линкуем тесты с GoogleTest
# gtest_main автоматически добавляет main функцию для тестов
target_link_libraries(ipc_tests PRIVATE GTest::gtest_main)

# Включаем common.h из директории src
# ${CMAKE_SOURCE_DIR} указывает на корень проекта (ipc_project)
target_include_directories(ipc_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/)

# Обнаружение и добавление тестов в CTest
include(GoogleTest)
gtest_discover_tests(ipc_tests)