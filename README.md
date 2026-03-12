# C++ Multithreaded Web Server

A HTTP/1.1 web server built from scratch in C++17 using POSIX sockets, focused on concurrency and resource safety.

![Build](https://github.com/bbaannee/cpp-webserver/actions/workflows/build.yml/badge.svg)

## Features

- **Thread Pool** — spawns one worker thread per CPU core, handles concurrent connections efficiently
- **RAII Socket Management** — custom `UniqueSocket` with deleter ensures sockets are always closed, no leaks
- **Singleton Logger** — thread-safe logging with mutex, writes timestamped entries to `server.log`
- **MIME Type Detection** — serves HTML, CSS, JS, PNG, JPEG, WebP, SVG, JSON and more
- **Directory Traversal Protection** — sanitizes request paths to prevent `../../etc/passwd` attacks
- **Binary-safe File Serving** — images and other binary files served correctly via `vector<char>`
- **SO_REUSEADDR** — port is freed immediately after server restart, no "address already in use" errors

## How to Run

### Linux / macOS

```bash
cmake -B build -S .
cmake --build build
./build/server
```

Then open `http://localhost:8080` in your browser.

### With Make (Linux only)

```bash
make
./server
```

## Project Structure

```
.
├── include/
│   ├── Logger.h        # Thread-safe singleton logger
│   ├── TcpListener.h   # TCP socket wrapper (bind, listen, accept)
│   └── WebServer.h     # HTTP server with thread pool
├── src/
│   ├── Logger.cpp
│   ├── TcpListener.cpp
│   ├── WebServer.cpp
│   └── main.cpp
├── www/                # Static files served by the server
│   └── index.html
└── CMakeLists.txt
```

## How It Works

```
main() → WebServer::start()
              ↓
     TcpListener::init()       — creates and binds socket
              ↓
     spawn N worker threads    — one per CPU core
              ↓
     accept loop               — accepts connections, pushes to queue
              ↓
     worker threads            — pick from queue, call handleClient()
              ↓
     handleClient()            — parse HTTP request, read file, send response
```

## Notes

> This server uses POSIX sockets and is designed for **Linux and macOS**.
> Windows is not supported without a POSIX compatibility layer (e.g. WSL).
