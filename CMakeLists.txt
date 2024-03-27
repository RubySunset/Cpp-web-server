cmake_minimum_required(VERSION 3.5.0)
project(Cpp-web-server VERSION 0.1.0 LANGUAGES C CXX)


include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_library(Sources ${SOURCES})

# add_executable(Cpp-web-server ${SOURCES})

add_executable(server "main.cpp")
add_executable(simple "simple_server.cpp")
add_executable(tests "tests/tests.cpp")
add_executable(bench "tests/benchmark.cpp")

target_link_libraries(server Sources)
target_link_libraries(simple Sources)
target_link_libraries(tests Sources)
target_link_libraries(bench Sources)
set_target_properties(server simple tests bench PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

include(CTest)
enable_testing()

