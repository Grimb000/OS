cmake_minimum_required(VERSION 3.10)
project(ArrayProcessor)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES_CPP98 
    src/cpp98/array_processor_cpp98.cpp
)

set(SOURCES_CPP14 
    src/cpp14/array_processor_cpp14.cpp
)

add_library(array_processor_lib_cpp98 STATIC ${SOURCES_CPP98})
set_target_properties(array_processor_lib_cpp98 PROPERTIES CXX_STANDARD 98)

add_library(array_processor_lib_cpp14 STATIC ${SOURCES_CPP14})
set_target_properties(array_processor_lib_cpp14 PROPERTIES CXX_STANDARD 14)

add_executable(array_processor_cpp98 src/cpp98/main_cpp98.cpp)
target_link_libraries(array_processor_cpp98 array_processor_lib_cpp98)
set_target_properties(array_processor_cpp98 PROPERTIES CXX_STANDARD 98)

add_executable(array_processor_cpp14 src/cpp14/main_cpp14.cpp)
target_link_libraries(array_processor_cpp14 array_processor_lib_cpp14)
set_target_properties(array_processor_cpp14 PROPERTIES CXX_STANDARD 14)

add_executable(test_cpp98 tests/cpp98/test_cpp98.cpp)
target_link_libraries(test_cpp98 array_processor_lib_cpp98)
set_target_properties(test_cpp98 PROPERTIES CXX_STANDARD 98)

add_executable(test_cpp14 tests/cpp14/test_cpp14.cpp)
target_link_libraries(test_cpp14 array_processor_lib_cpp14)
set_target_properties(test_cpp14 PROPERTIES CXX_STANDARD 14)

add_custom_target(run_tests 
    COMMAND test_cpp98
    COMMAND test_cpp14
    DEPENDS test_cpp98 test_cpp14
    COMMENT "Running all tests..."
)
