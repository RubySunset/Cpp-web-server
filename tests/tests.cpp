#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>
#include <cstring>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <atomic>

#include "server.h"

const int TEST_PORT = 8080;
const std::string SERVER_ADDRESS = "127.0.0.1";

extern std::atomic<int> running;

std::string sendRequest(const std::string& method, const std::string& path, const std::string& body = "") {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != -1);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TEST_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS.c_str(), &server_addr.sin_addr);

    assert(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) >= 0);

    std::stringstream request;
    request << method << " " << path << " HTTP/1.1\r\n";
    request << "Host: " << SERVER_ADDRESS << "\r\n";
    if (!body.empty()) {
        request << "Content-Length: " << body.length() << "\r\n";
    }
    request << "\r\n" << body;

    std::string request_str = request.str();
    send(sock, request_str.c_str(), request_str.length(), 0);
    shutdown(sock, SHUT_WR);

    char buffer[4096] = {0};
    std::string response;
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, bytes_received);
    }

    close(sock);
    return response;
}

void testGET() {
    std::string response = sendRequest("GET", "/");
    assert(response.find("HTTP/1.1 200 OK") != std::string::npos);
    assert(response.find("Content-Type: text/html") != std::string::npos);
    std::cout << "GET test passed\n";
}

void testPOST() {
    std::string body = "key1=value1&key2=value2";
    std::string response = sendRequest("POST", "/test", body);
    assert(response.find("HTTP/1.1 200 OK") != std::string::npos);
    assert(response.find("Received POST data: " + body) != std::string::npos);
    std::cout << "POST test passed\n";
}

void test404() {
    std::string response = sendRequest("GET", "/nonexistent");
    assert(response.find("HTTP/1.1 404 Not Found") != std::string::npos);
    std::cout << "404 test passed\n";
}

void testHEAD() {
    std::string response = sendRequest("HEAD", "/");
    assert(response.find("HTTP/1.1 200 OK") != std::string::npos);
    assert(response.find("Content-Type: text/html") != std::string::npos);
    assert(response.find("\r\n\r\n") == response.length() - 4);
    std::cout << "HEAD test passed\n";
}

void testOPTIONS() {
    std::string response = sendRequest("OPTIONS", "*");
    assert(response.find("HTTP/1.1 200 OK") != std::string::npos);
    assert(response.find("Allow: GET, POST, PUT, DELETE, OPTIONS, HEAD") != std::string::npos);
    std::cout << "OPTIONS test passed\n";
}

int main() {
    std::thread server_thread([]() {
        HTTPServer server(TEST_PORT);
        server.run();
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    testGET();
    testPOST();
    test404();
    testHEAD();
    testOPTIONS();

    running = false;
    server_thread.join();

    std::cout << "All tests passed!\n";
    return 0;
}