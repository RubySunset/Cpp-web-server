#pragma once

#include <string>
#include <unordered_map>

class SimpleServer {
public:
    SimpleServer();
    ~SimpleServer();
    void run();
    void stop();

private:
    int port = 8080;
    int server_fd;
    std::unordered_map<std::string, std::string> routes;

    void handle_connection(int client_socket);
    void send_response(int client_socket, const std::string& content, int status_code);
};