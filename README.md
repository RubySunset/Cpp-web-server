# Web Server

A HTTP server written in C++, using POSIX sockets (hence, will only work on POSIX-compliant operating systems).

- Features of HTTP/1.1, including the ability to serve static files, handle custom endpoints (e.g. to generate dynamic pages), extract query parameters, different HTTP methods, error codes, and more.
- Uses a combination of a thread pool and epoll instances. Client requests are distributed among threads, each of which uses epoll to handle very large numbers of simultaneous connections.
    - Thus scalable to many cores, yet remains performant even on modest hardware.
- Implements an in-memory cache to store frequently accessed files, shared between all threads in the thread pool using multiple-reader single-writer semantics.
- Functional tests to verify correctness of overall system for a representative set of requests.
- Benchmarked at about 10x faster than the naive un-optimised solution when receiving 100,000 simple HTTP requests.

## Structure

- `include` contains all header files.
- `src` contains all source files.
- `serve` is the root directory for HTTP requests. Note that in its current state, the web server does not provide protection against path traversal attacks.
- `tests` currently contains two files of interest:
    - `tests.cpp` - a suit of functional tests.
    - `benchmark.cpp` - a benchmark that simulates multiple clients (on different threads) sending many consecutive requests to the server running at `localhost:8080`. Parameters can be tuned.
- `main.cpp` runs the actual server.
- `src/simple_server.cpp` runs a very simple server with little functionality and no optimisations (a blocking server socket to accept connections, which are immediately handled in a serial manner). Its primary use is as a comparison to the optimised version for benchmarking.