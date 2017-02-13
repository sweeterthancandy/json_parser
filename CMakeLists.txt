
cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-std=c++14")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

aux_source_directory(test test_sources)

find_package(GTest REQUIRED)

include_directories(json_parser)

add_executable( json_parser_tests ${test_sources} )

target_link_libraries(json_parser_tests GTest::GTest GTest::Main)

