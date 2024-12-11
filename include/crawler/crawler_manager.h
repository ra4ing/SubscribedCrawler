#ifndef __CRAWLER_MANAGER_H__
#define __CRAWLER_MANAGER_H__

#include <unordered_set>
#include <memory>
#include <atomic>
#include "./crawler.h"
#include "./url_queue.h"
#include "./http_client.h"
#include "./parser.h"
#include "./cache.h"
#include "../thread_pool.h"
#include "../logger.h"
#include "../config.h"
#include "../common_types.h"

class CrawlerManager : public Crawler {
public:
    explicit CrawlerManager();
    ~CrawlerManager();

    CrawlerStats Crawl(const std::string& start_url) override;
    CrawlerStats GetStats() const override;
    void Shutdown();

private:
    void WorkerFunction();

    std::shared_ptr<Config> config_;
    std::shared_ptr<UrlQueue> url_queue_;
    std::shared_ptr<Cache> cache_;
    std::shared_ptr<Logger> logger_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<HttpClient> http_client_;
    std::unique_ptr<Parser> parser_;

    std::atomic<bool> stop_;
    std::unordered_set<std::string> visited_urls_;
    std::mutex visited_mutex_;
};

#endif // __CRAWLER_MANAGER_H__