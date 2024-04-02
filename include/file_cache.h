#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iostream>

class FileCache {
public:
    FileCache(size_t max_size = 1024 * 1024 * 10);  // Default 10MB cache
    std::string get(const std::string& path);
    void put(const std::string& path, const std::string& content);

private:
    struct CacheEntry {
        std::string content;
        std::chrono::steady_clock::time_point last_access;
    };

    std::unordered_map<std::string, CacheEntry> cache;
    std::shared_mutex cache_mutex;
    size_t max_size;
    size_t current_size;

    void evict();
};