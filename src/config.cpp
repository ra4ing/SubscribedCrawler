#include "config.h"

Config::Config()
    : max_concurrency_(16),
    request_timeout_(5 * 1000),
    crawl_timeout_(10 * 60 * 1000),
    user_agent_(""),
    cache_directory_("../cache"),
    log_file_dir_("../logs/search.log")
{
}

Config::~Config() = default;

std::shared_ptr<Config> Config::GetConfig()
{
    static std::shared_ptr<Config> instance = std::make_shared<Config>();
    return instance;
}

const int Config::getMaxConcurrency() const
{
    return max_concurrency_;
}

const size_t Config::getRequestTimeout() const
{
    return request_timeout_;
}

const size_t Config::getCrawlTimeOut() const
{
    return crawl_timeout_;
}

const std::string& Config::getUserAgent() const
{
    return user_agent_;
}

const std::string& Config::getCacheDirectory() const
{
    return cache_directory_;
}


const std::string& Config::getLogFileDir() const
{
    return log_file_dir_;
}





