cmake_minimum_required(VERSION 3.17)
project(reply2018_practice)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -s -O3")
set(CMAKE_CXX_STANDARD 14)
add_definitions(-DCLION)

add_executable(reply2018_practice main.cpp)