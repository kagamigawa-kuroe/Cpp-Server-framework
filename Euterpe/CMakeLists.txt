cmake_minimum_required(VERSION 3.20)
project(Euterpe)

set(CMAKE_CXX_STANDARD 20)

include_directories("/usr/local/include")
include_directories("/usr/local/lib")

add_executable(Euterpe tests/log_test.cpp src/Log/log.cpp src/Log/log.h src/utils/utils.h src/utils/utils.cpp src/utils/singleton.h src/config/config.h tests/config_test.cpp src/config/config.cpp src/thread/euterpe_thread.h src/utils/noncopyable.h tests/thread_test.cpp src/thread/euterpe_thread.cpp src/euterpe.h src/thread/mutex.h src/thread/mutex.cpp tests/lock_test.cpp src/utils/macro.h tests/test_assert.cpp src/coroutines/fiber.h src/coroutines/fiber.cpp tests/fiber_test.cpp src/scheduler/scheduler.cpp src/scheduler/scheduler.h tests/test_scheduler.cpp src/IO/IoManager.h src/IO/IoManager.cpp tests/test_iomanager.cpp)

target_link_libraries(Euterpe /usr/local/lib/libyaml-cpp.so)