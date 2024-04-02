#include "server.h"

std::atomic<bool> running(true);

void signal_handler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    running = false;
}

HTTPServer::HTTPServer(int port)
    : PORT(port), 
      file_cache(1024 * 1024 * 50),  // 50MB cache
      method_handler(file_cache),
    //   thread_pool(&method_handler, std::thread::hardware_concurrency())
      thread_pool(&method_handler, std::thread::hardware_concurrency())
{
    server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd == 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("Failed to set socket options");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        throw std::runtime_error("Failed to listen");
    }

    signal(SIGINT, signal_handler);
}

HTTPServer::~HTTPServer() {
    close(server_fd);
}

void HTTPServer::stop() {
    running = false;
}

void HTTPServer::run() {
    while (running) {
        int client_socket = accept4(server_fd, NULL, NULL, SOCK_NONBLOCK);
        if (client_socket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                std::cerr << "Accept error" << std::endl;
                continue;
            }
        }
        thread_pool.delegate_client(client_socket);
    }
    std::cout << "Server shutting down...\n";
}