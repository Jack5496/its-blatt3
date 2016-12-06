cmake_minimum_required(VERSION 3.6)
project(joarndt)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    pa3_client.c
    pa3_client.h
    pa3_server.c
    pa3_server.h)

add_executable(joarndt ${SOURCE_FILES})