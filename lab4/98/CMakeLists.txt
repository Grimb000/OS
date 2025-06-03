cmake_minimum_required(VERSION 3.14) # FetchContent требует 3.11+, 3.14+ для некоторых улучшений
project(IPC_Project VERSION 1.0 LANGUAGES CXX)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(gtest_force_shared_crt ON CACHE BOOL "Force GoogleTest to use shared (DLL) CRT" FORCE)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


# Установка стандарта C++98
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED False)
set(CMAKE_CXX_EXTENSIONS False)


# Добавляем поддиректорию с исходным кодом
add_subdirectory(src)

# Настройка Google Test
# Подключаем FetchContent для загрузки GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip # Используйте актуальный релиз
  # Для полной воспроизводимости можно указать GIT_REPOSITORY и GIT_TAG
  # GIT_REPOSITORY https://github.com/google/googletest.git
  # GIT_TAG release-1.14.0
)

# Указываем, что GoogleTest должен быть доступен только во время сборки,
# и не должен устанавливаться или экспортироваться.
FetchContent_MakeAvailable(googletest)

# Включаем тестирование с помощью CTest
enable_testing()

# Добавляем поддиректорию с тестами
add_subdirectory(tests)

# Сообщение о завершении конфигурации
message(STATUS "CMake configuration finished. Receiver and Sender executables will be built.")
message(STATUS "Run 'ctest' in the build directory to execute tests after building.")
