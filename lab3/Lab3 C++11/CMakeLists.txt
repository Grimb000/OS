# CMakeList.txt: проект CMake для Lab3; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.11)
include(FetchContent)
# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Lab3_11")

# Подключение GoogleTest через FetchContent

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Lab3_11 "src/main.cpp" "src/marker.cpp" "src/marker.h" "src/MarkerData.h")
target_include_directories(Lab3_11 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Lab3_11 PROPERTY CXX_STANDARD 20)
endif()

# Тесты
add_executable(test_marker "tests/test_marker.cpp" "src/marker.cpp" "src/marker.h" "src/MarkerData.h")
target_link_libraries(test_marker PRIVATE gtest gtest_main)
target_include_directories(test_marker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

enable_testing()
add_test(NAME test_marker COMMAND test_marker)