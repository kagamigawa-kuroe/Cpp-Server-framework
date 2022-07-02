cmake_minimum_required(VERSION 3.20)
project(Euterpe)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(Euterpe
        src/log.cpp
        src/log.h)
