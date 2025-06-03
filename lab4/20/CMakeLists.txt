# CMakeList.txt: проект CMake для 20; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(gtest_force_shared_crt ON CACHE BOOL "Force GoogleTest to use shared (DLL) CRT" FORCE)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

cmake_minimum_required(VERSION 3.14) # FetchContent и другие возможности
project(IPC_Project_Modern VERSION 2.0 LANGUAGES CXX)

# Установка стандарта C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False) # Отключаем GNU расширения

# Для Windows, если используем MinGW, можно указать статические библиотеки времени выполнения
# if(CMAKE_COMPILER_IS_GNUCXX AND WIN32)
#   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
# endif()



include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip # Или более новый тег
  # Для полной воспроизводимости:
  # GIT_REPOSITORY https://github.com/google/googletest.git
  # GIT_TAG release-1.14.0 
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Добавляем поддиректории
add_subdirectory(src)
add_subdirectory(tests)

message(STATUS "Modern IPC Project CMake configuration finished.")
message(STATUS "Executables: ReceiverModern, SenderModern.")
message(STATUS "Run 'ctest' in the build directory to execute tests after building.")
