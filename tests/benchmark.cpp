#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "server.h"
#include "simple_server.h"

const int PORT = 8080;
const std::string SERVER_ADDRESS = "127.0.0.1";
const int NUM_THREADS = 100;
const int REQUESTS_PER_THREAD = 1000;
std::atomic<int> successful_requests(0);
std::atomic<int> failed_requests(0);

void sendRequests() {
    for (int i = 0; i < REQUESTS_PER_THREAD; ++i) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            failed_requests++;
            continue;
        }

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_ADDRESS.c_str(), &server_addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            failed_requests++;
            continue;
        }

        const char* request = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
        send(sock, request, strlen(request), 0);
        shutdown(sock, SHUT_WR);

        char buffer[1024] = {0};
        if (recv(sock, buffer, sizeof(buffer), 0) > 0) {
            successful_requests++;
        } else {
            failed_requests++;
        }

        close(sock);
    }
}

int main() {
    HTTPServer server;
    // SimpleServer server;
    std::thread server_thread([&server]{
        server.run();
    });

    std::vector<std::thread> threads;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(sendRequests);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    int total_requests = NUM_THREADS * REQUESTS_PER_THREAD;
    double seconds = duration.count() / 1000.0;
    double requests_per_second = total_requests / seconds;

    std::cout << "Benchmark Results:\n";
    std::cout << "Total requests: " << total_requests << "\n";
    std::cout << "Successful requests: " << successful_requests << "\n";
    std::cout << "Failed requests: " << failed_requests << "\n";
    std::cout << "Time taken: " << seconds << " seconds\n";
    std::cout << "Requests per second: " << requests_per_second << "\n";

    server.stop();
    server_thread.join();

    return 0;
}