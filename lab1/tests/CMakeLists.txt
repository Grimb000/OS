cmake_minimum_required(VERSION 3.8)

project(Tests)

# Включаем поддержку тестирования
enable_testing()

# Ищем Google Test
find_package(GTest QUIET)

# Если GTest не найден, загружаем его через FetchContent
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Убедимся, что GTest использует общую CRT (только для Windows)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Добавляем исполняемый файл для тестов
add_executable(Tests
    tests.cpp
    ${CMAKE_SOURCE_DIR}/Main/main.cpp
    ${CMAKE_SOURCE_DIR}/Creator/creator.cpp
    ${CMAKE_SOURCE_DIR}/Reporter/reporter.cpp
)

# Подключаем заголовочные файлы
target_include_directories(Tests PRIVATE
    ${CMAKE_SOURCE_DIR}/Shared
    ${CMAKE_SOURCE_DIR}/Main
    ${CMAKE_SOURCE_DIR}/Creator
    ${CMAKE_SOURCE_DIR}/Reporter
)

# Линкуем с Google Test и другими необходимыми библиотеками
target_link_libraries(Tests PRIVATE
    gtest_main
)

# Добавляем тест в систему CTest
add_test(NAME EmployeeTests COMMAND Tests)