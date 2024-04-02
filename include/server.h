#pragma once

#include <atomic>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <filesystem>
#include <fstream>
#include <csignal>

#include "thread_pool.h"
#include "file_cache.h"
#include "request_parser.h"
#include "response_builder.h"
#include "http_method_handler.h"

class HTTPServer {
public:
    HTTPServer(int port = 8080);
    ~HTTPServer();
    void stop();

    void run();

private:
    int server_fd;
    struct sockaddr_in address;
    const int PORT;
    FileCache file_cache;
    HTTPMethodHandler method_handler;
    ThreadPool thread_pool;
};