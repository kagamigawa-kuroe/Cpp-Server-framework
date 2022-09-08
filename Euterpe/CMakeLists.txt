cmake_minimum_required(VERSION 3.20)
project(Euterpe)

set(CMAKE_CXX_STANDARD 20)

include_directories("/usr/local/include")
include_directories("/usr/local/lib")

add_executable(Euterpe tests/log_test.cpp src/Log/log.cpp src/Log/log.h src/utils/utils.h src/utils/utils.cpp src/utils/singleton.h src/config/config.h tests/config_test.cpp src/config/config.cpp src/thread/euterpe_thread.h src/utils/noncopyable.h tests/thread_test.cpp src/thread/euterpe_thread.cpp src/euterpe.h src/thread/mutex.h src/thread/mutex.cpp tests/lock_test.cpp src/utils/macro.h tests/test_assert.cpp src/coroutines/fiber.h src/coroutines/fiber.cpp tests/fiber_test.cpp src/scheduler/scheduler.cpp src/scheduler/scheduler.h tests/test_scheduler.cpp src/IO/IoManager.h src/IO/IoManager.cpp tests/test_iomanager.cpp src/time/Timer.cpp src/time/Timer.h tests/timer.cpp src/hook/hook.cpp src/hook/hook.h tests/test_hook.cpp src/fd_manager/fd_manager.cpp src/fd_manager/fd_manager.h src/Network_Base/address.cpp src/Network_Base/address.h src/utils/endian.h tests/test_ip_addr.cpp src/Network_Base/Socket.cpp src/Network_Base/Socket.h tests/test_socket.cpp src/ByteArray/ByteArray.cpp src/ByteArray/ByteArray.h tests/test_bytearry.cpp src/http/http.h src/http/http.cpp src/http/http_parser.cpp src/http/http_parser.h src/http/http11_common.h src/http/http11_parser.h src/http/http11_parser.rl.cpp src/http/httpclient_parser.h src/http/httpclient_parser.rl.cpp tests/http.test.cpp src/tcp_server/tcp_server.cpp src/tcp_server/tcp_server.h tests/test_tcp_server.cpp src/stream/Stream.cpp src/stream/Stream.h src/stream/SocketStream.cpp src/stream/SocketStream.h src/http/HttpSession.cpp src/http/HttpSession.h src/HttpServer/HttpServer.cpp src/HttpServer/HttpServer.h tests/test_http_server.cpp src/servlet/Servlet.cpp src/servlet/Servlet.h)

target_link_libraries(Euterpe /usr/local/lib/libyaml-cpp.so)