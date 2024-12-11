#ifndef __CRAWLER_H__
#define __CRAWLER_H__

#include <string>
#include <atomic>
#include "../common_types.h"

class Crawler
{
public:
    Crawler() = default;
    virtual ~Crawler() = default;

    Crawler(const Crawler&) = delete;
    Crawler& operator=(const Crawler&) = delete;

    virtual CrawlerStats Crawl(const std::string& start_url) = 0;
    virtual CrawlerStats GetStats() const = 0; 

protected:
    std::atomic<int> urls_fetched_{ 0 };
    std::atomic<int> urls_parsed_{ 0 };
    std::atomic<int> urls_failed_{ 0 };
};


#endif // __CRAWLER_H__