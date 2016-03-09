cmake_minimum_required(VERSION 2.8.4)
project(NumeralSystemsConverter)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99") # seems like it doesn't work in clion
set(CMAKE_CXX_FLAGS_DEBUG  "fwefewfwef")

set(SOURCE_FILES main.c)
add_executable(NumeralSystemsConverter ${SOURCE_FILES})
