cmake_minimum_required(VERSION 3.8)
project(mySSHserver)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp Parser.cpp Parser.h Exceptions.h Exceptions.cpp Evaluate.cpp Evaluate.h)
add_executable(mySSHserver ${SOURCE_FILES} Exceptions.h)