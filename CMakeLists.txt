cmake_minimum_required(VERSION 3.10)

project(TinyShader)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

message(STATUS "[CMakeDebug]Found root files: ${CMAKE_SOURCE_DIR}")

message(STATUS "[CMakeDebug]Found source files: ${SRC}")

add_executable(TinyShader ${SRC})

target_include_directories(TinyShader PUBLIC "${CMAKE_SOURCE_DIR}/include")