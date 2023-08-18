# Cpp-Server-framework
### Cpp-Server-framework

---

#### Main Features
This markdown document outlines the development of a C++ server framework from scratch, drawing inspiration from various design principles found in Java or Go frameworks, as well as leveraging existing open-source C++ frameworks and libraries such as Boost, YAZI, Sylar, and libco.

The envisioned functional modules and their current progress include:

- [x] Log System inspired by log4j
  - Modularized output of program runtime logs
  - Customizable output formats
- [x] Threading Module
  - Wrapper around pthread, providing similar functionality as C++11's thread library
  - Abstraction of locks and implementation of thread safety
  - Thread pool based on coroutine tasks
- [x] Coroutine Framework
  - Initial plans to leverage C++20's features; however, after comparing with other popular frameworks, the decision is made to implement a custom coroutine framework from the ground up
  - Utilizes the native ucontext.h library for Linux and implements a coroutine framework based on context switching
  - Support for coroutine scheduling based on a thread pool
- [x] Configuration Framework
  - Adopts a configuration file approach similar to the Spring framework 
  - Utilizes YAML configuration files to manage configuration for different modules
- [x] I/O Framework
  - Extends the coroutine scheduling framework and integrates epoll to manage socket connections
- [x] Timer Module
  - Implements a basic timed task management system for simple addition and removal of tasks
  - Combines with the I/O scheduling module for advanced I/O event triggering and execution
- [x] Hook Module
  - Primarily used to extend native library functions, offering more detailed customization
  - The hook operates at the thread level, with individual thread-specific switches for activation
  - Rewrites system read and write functions, implementing asynchronous processing through timers and epoll to reduce blocking time and enhance performance
- [x] Socket Abstraction
  - Object-oriented wrapper around the native socket API, providing a cleaner socket API
  - Simplified encapsulation of a TCP server
  - HTTP server class encapsulation, including HTTP protocol parsing and rapid API-based setup of an HTTP server
  - Implementation of URL redirection similar to servlets
- [ ] RPC Module
  - Bottom-up implementation of an RPC (Remote Procedure Call) interface
- [ ] MySQL/Redis Drivers and ZooKeeper Wrapper

---

#### Structure of project

```bash
./Euterpe/src
├── ByteArray
│   ├── ByteArray.cpp
│   └── ByteArray.h
├── config
│   ├── config.cpp
│   ├── config.h
│   └── config.md
├── coroutines
│   ├── fiber.cpp
│   ├── fiber.h
│   └── fiber.md
├── euterpe.h
├── fd_manager
│   ├── fd_manager.cpp
│   └── fd_manager.h
├── hook
│   ├── hook.cpp
│   └── hook.h
├── http
│   ├── http11_common.h
│   ├── http11_parser.h
│   ├── http11_parser.rl
│   ├── http11_parser.rl.cpp
│   ├── httpclient_parser.h
│   ├── httpclient_parser.rl
│   ├── httpclient_parser.rl.cpp
│   ├── http.cpp
│   ├── http.h
│   ├── http_parser.cpp
│   ├── http_parser.h
│   ├── HttpSession.cpp
│   └── HttpSession.h
├── HttpServer
│   ├── HttpServer.cpp
│   └── HttpServer.h
├── image
│   └── Log_usgae.jpg
├── IO
│   ├── IoManager.cpp
│   └── IoManager.h
├── Log
│   ├── log.cpp
│   ├── log.h
│   └── Log_note.md
├── Network_Base
│   ├── address.cpp
│   ├── address.h
│   ├── Socket.cpp
│   └── Socket.h
├── scheduler
│   ├── scheduler.cpp
│   ├── scheduler.h
│   └── Scheduler.md
├── stream
│   ├── SocketStream.cpp
│   ├── SocketStream.h
│   ├── Stream.cpp
│   └── Stream.h
├── tcp_server
│   ├── tcp_server.cpp
│   └── tcp_server.h
├── thread
│   ├── euterpe_thread.cpp
│   ├── euterpe_thread.h
│   ├── mutex.cpp
│   ├── mutex.h
│   └── Thread.md
├── time
│   ├── Timer.cpp
│   └── Timer.h
└── utils
    ├── endian.h
    ├── macro.h
    ├── noncopyable.h
    ├── singleton.h
    ├── utils.cpp
    └── utils.h

```
