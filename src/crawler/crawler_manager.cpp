#include <iostream>
#include <boost/url.hpp>
#include "../../include/crawler/crawler_manager.h"
#include "../../include/logger.h"
#include "../../include/config.h"

CrawlerManager::CrawlerManager()
    : config_(Config::GetConfig()),
    url_queue_(std::make_shared<UrlQueue>()),
    cache_(std::make_shared<Cache>()),
    logger_(Logger::GetLogger()),
    thread_pool_(std::make_unique<ThreadPool>(config_->getMaxConcurrency())),
    http_client_(std::make_unique<HttpClient>()),
    parser_(std::make_unique<Parser>()),
    stop_(true) {
}

CrawlerManager::~CrawlerManager() {

}

CrawlerStats CrawlerManager::Crawl(const std::string& start_url) {
    if (!stop_) {
        logger_->Error("Crawler is already running.");
        return CrawlerStats{ -1, -1, -1 };
    }

    stop_ = false;
    url_queue_->Push(start_url);

    for (size_t i = 0; i < config_->getMaxConcurrency(); ++i) {
        thread_pool_->Enqueue([this]() { WorkerFunction(); });
    }
    std::thread timer_thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(Config::GetConfig()->getCrawlTimeOut()));
        Shutdown();
        });

    if (timer_thread.joinable()) {
        timer_thread.join();
    }

    return GetStats();
}

CrawlerStats CrawlerManager::GetStats() const {
    CrawlerStats stats;
    stats.urls_fetched = urls_fetched_.load();
    stats.urls_parsed = urls_parsed_.load();
    stats.urls_failed = urls_failed_.load();
    return stats;
}

void CrawlerManager::WorkerFunction() {
    while (!stop_) {
        auto url_opt = url_queue_->Pop();
        if (!url_opt.has_value()) {
            if (!stop_) logger_->Warn("No url in queue");
            break;
        }

        std::string url = url_opt.value();

        {
            std::lock_guard<std::mutex> lock(visited_mutex_);
            if (visited_urls_.find(url) != visited_urls_.end()) {
                continue;
            }
            visited_urls_.insert(url);
        }

        // 检查缓存
        auto cached_content = cache_->Get(url);
        // if (cached_content.has_value()) {
        //     ++urls_fetched_;
        //     auto parsed = parser_->ParseLinks(cached_content.value(), url);
        //     urls_parsed_ += parsed.links.size();
        //     for (const auto& link : parsed.links) {
        //         url_queue_->Push(link);
        //     }
        //     continue;
        // }

        // 发起 HTTP 请求
        FetchResult fetch_result{ false, "", 0 };
        http_client_->Fetch(
            fetch_result,
            url,
            config_->getUserAgent(),
            config_->getRequestTimeout()
        );

        if (fetch_result.success) {
            ++urls_parsed_;
            // cache_->Put(url, fetch_result.body);
            if (url.find("spiderbuf") == url.npos) {
                return;
            }
            auto parsed = parser_->ParseLinks(fetch_result.body, url);
            logger_->Info("Parsed url: " + url);

            urls_fetched_ += parsed.links.size();
            for (const auto& link : parsed.links) {
                url_queue_->Push(link);
            }
        }
        else {
            ++urls_failed_;
        }
    }
}

void CrawlerManager::Shutdown() {
    logger_->Warn("Stopping crawler...");
    url_queue_->Shutdown();
    stop_ = true;
    thread_pool_->Shutdown();
    std::cout << "CrawlerManager shutdown" << std::endl;
}