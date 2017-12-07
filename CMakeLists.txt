cmake_minimum_required(VERSION 3.8)
project(mySSHserver)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp Parser.cpp Parser.h)
add_executable(mySSHserver ${SOURCE_FILES})