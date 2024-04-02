#include "file_cache.h"

FileCache::FileCache(size_t max_size) : max_size(max_size), current_size(0) {}

std::string FileCache::get(const std::string& path) {
    std::shared_lock<std::shared_mutex> lock(cache_mutex);
    auto it = cache.find(path);
    if (it != cache.end()) {
        it->second.last_access = std::chrono::steady_clock::now();
        return it->second.content;
    }
    return "";
}

void FileCache::put(const std::string& path, const std::string& content) {
    std::unique_lock<std::shared_mutex> lock(cache_mutex);
    
    while (current_size + content.size() > max_size && !cache.empty()) {
        evict();
    }

    if (content.size() <= max_size) {
        cache[path] = {content, std::chrono::steady_clock::now()};
        current_size += content.size();
    }
}

void FileCache::evict() {
    auto oldest = std::min_element(cache.begin(), cache.end(),
        [](const auto& a, const auto& b) {
            return a.second.last_access < b.second.last_access;
        });
    
    if (oldest != cache.end()) {
        current_size -= oldest->second.content.size();
        cache.erase(oldest);
    }
}