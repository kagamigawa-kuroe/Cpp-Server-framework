cmake_minimum_required(VERSION 3.20)
project(Euterpe)

set(CMAKE_CXX_STANDARD 20)

add_executable(Euterpe tests/log_test.cpp src/Log/log.cpp src/Log/log.h src/utils/utils.h src/utils/utils.cpp src/utils/singleton.h)
