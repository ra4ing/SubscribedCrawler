#ifndef __CACHE_H__
#define __CACHE_H__

#include <string>
#include <optional>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>


class Cache {
public:
    Cache();
    ~Cache();

    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    std::optional<std::string> Get(const std::string& url);
    void Put(const std::string& url, const std::string& content);

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> cache_map_;
};
#endif // __CACHE_H__