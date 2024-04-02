#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cassert>

#include "http_method_handler.h"

#define MAX_EVENTS 1024

class Worker {
public:
    Worker(HTTPMethodHandler *method_handler);
    ~Worker();
    void add_fd(int fd);
private:
    void read_msg(int fd);
    void handle_client(int fd);

    std::thread thread;
    int epoll_fd, num_fds = 0;
    std::vector<int> fd_buf;
    std::mutex fd_buf_mut;
    std::condition_variable fd_buf_cv;
    std::unordered_map<int, std::string> client_buf;
    HTTPMethodHandler *method_handler;
    bool stop = false;
};

class ThreadPool {
public:
    ThreadPool(HTTPMethodHandler *method_handler, int threadpool_size);
    void delegate_client(int fd);
private:
    int threadpool_size, next_worker = 0;
    std::vector<std::unique_ptr<Worker>> workers;
};