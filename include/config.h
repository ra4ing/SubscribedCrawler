#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <memory>
#include <chrono>

class Config {
public:
    Config();
    ~Config();

    // Not copyable or movable
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    static std::shared_ptr<Config> GetConfig();

    const int getMaxConcurrency() const;
    const size_t getRequestTimeout() const;
    const size_t getCrawlTimeOut() const;
    const std::string& getUserAgent() const;
    const std::string& getCacheDirectory() const;
    const std::string& getLogFileDir() const;

private:
    int max_concurrency_;
    size_t request_timeout_;
    size_t crawl_timeout_;
    std::string user_agent_;
    std::string cache_directory_;
    std::string log_file_dir_;
    
};

#endif // __CONFIG_H__