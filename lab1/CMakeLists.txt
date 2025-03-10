cmake_minimum_required(VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("Lab1")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Глобальный путь для включаемых файлов
include_directories("${PROJECT_SOURCE_DIR}/Shared")

# Добавляем поддиректории для исполняемых файлов
add_subdirectory("Creator")
add_subdirectory("Reporter")
add_subdirectory("Main")

# Настройка тестирования
enable_testing()

# Загружаем Google Test (если не найден)
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
endif()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Добавляем поддиректорию для тестов
add_subdirectory("tests")