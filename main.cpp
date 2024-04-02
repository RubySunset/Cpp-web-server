#include <iostream>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "include/server.h"

void send_request() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    std::cout << "connected to server" << std::endl;

    std::string request_str = "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: YourClient/1.0\r\nAccept: */*\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    send(sock, request_str.c_str(), request_str.length(), 0);
    shutdown(sock, SHUT_WR);
    std::cout << "send request" << std::endl;

    char buffer[4096];
    std::string response;
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, bytes_received);
    }
    std::cout << "got response" << std::endl;

    close(sock);
    std::cout << response;

}

int main() {
    try {
        HTTPServer server;
        std::cout << "Server listening on port 8080" << std::endl;
        // std::thread server_thread([&server]{
        //     server.run();
        // });
        // send_request();
        server.run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}