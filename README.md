# Cpp-web-server

A HTTP server written in C++, using POSIX sockets (hence, will only work on POSIX-compliant operating systems).

- Features of HTTP/1.1, including the ability to serve static files, create custom endpoints, extract query parameters, different HTTP methods, error codes, and more.
- Many optimisations, including:
    - Use of epoll to monitor activity on many sockets (many clients) at once.
    - Use of a thread pool to handle client requests concurrently.
    - An in-memory cache to store frequently accessed pages, to avoid frequent filesystem accesses.
- Benchmarked at about 10x faster than the naive un-optimised solution when receiving 100,000 simple HTTP requests.

## Structure

- `include` contains all of the header (.h) files.
- `src` contains all of the source (.cpp) files.
- `serve` is the root directory for HTTP requests.
- `tests` currently contains two files of interest:
    - `tests.cpp` - a suit of functional tests.
    - `benchmark.cpp` - a benchmark that simulates 100 clients sending 1000 requests to the server running at `localhost:8080` (these parameters can be tuned).
- `main.cpp` runs the actual server.
- `simple_server.cpp` runs a very simple server with little functionality and no optimisations. Its primary use is as a comparison to the optimised version for benchmarking.