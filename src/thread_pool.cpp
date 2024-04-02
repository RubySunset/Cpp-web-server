#include "thread_pool.h"

Worker::Worker(HTTPMethodHandler *method_handler) : method_handler(method_handler) {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }
    thread = std::thread([this](){
        epoll_event events[MAX_EVENTS];
        while (!stop) {
            std::unique_lock<std::mutex> lock(fd_buf_mut);
            fd_buf_cv.wait(lock, [this](){
                return !fd_buf.empty() || num_fds > 0 || stop;
            });
            if (stop) {
                break;
            }
            for (int fd : fd_buf) {
                epoll_event event;
                event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                event.data.fd = fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
                    throw std::runtime_error("Failed to add file descriptor to epoll");
                }
                ++num_fds;
            }
            fd_buf.clear();
            lock.unlock();
            int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
            if (nfds == -1) {
                std::cerr << "Epoll wait error" << std::endl;
                continue;
            }
            for (int i = 0; i < nfds; ++i) {
                int fd = events[i].data.fd;
                read_msg(fd);
                if (events[i].events & EPOLLRDHUP) {
                    handle_client(fd);
                }
            }
        }
    });
}

Worker::~Worker() {
    stop = true;
    fd_buf_cv.notify_all();
    thread.join();
    close(epoll_fd);
}

void Worker::add_fd(int fd) {
    std::unique_lock<std::mutex> lock(fd_buf_mut);
    fd_buf.push_back(fd);
    lock.unlock();
    fd_buf_cv.notify_all();
}

void Worker::read_msg(int fd) {
    char buffer[4096];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        client_buf[fd].insert(client_buf[fd].end(), buffer, buffer + bytes_read);
    }
}

void Worker::handle_client(int fd) {
    HTTPRequest request = RequestParser::parse(client_buf[fd]);
    std::string response = method_handler->handleRequest(request);

    write(fd, response.c_str(), response.length());
    close(fd);
    --num_fds;
    client_buf.erase(fd);
}

ThreadPool::ThreadPool(HTTPMethodHandler *method_handler, int threadpool_size)
    : threadpool_size(threadpool_size)
{
    for (int i = 0; i < threadpool_size; ++i) {
        workers.push_back(std::make_unique<Worker>(method_handler));
    }
}

void ThreadPool::delegate_client(int fd) {
    workers[next_worker]->add_fd(fd);
    next_worker = (next_worker + 1) % threadpool_size;
}