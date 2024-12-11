#include "../../include/crawler/cache.h"


Cache::Cache() {}

Cache::~Cache() {}

std::optional<std::string> Cache::Get(const std::string& url) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = cache_map_.find(url);
    if (it != cache_map_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void Cache::Put(const std::string& url, const std::string& content) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    cache_map_[url] = content;
}
