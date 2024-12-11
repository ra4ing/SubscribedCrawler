#include <iostream>
#include <memory>
#include "../include/crawler/crawler_manager.h"
#include "../include/config.h"

int main() {
    std::string url = "https://www.spiderbuf.cn/playground/c01";
    // std::string url = "https://www.youtube.com";
    CrawlerManager crawler;
    auto stats = crawler.Crawl(url);

    std::cout << "Crawl Finished. Stats:\n"
        << "URLs Fetched: " << stats.urls_fetched << "\n"
        << "URLs Parsed: " << stats.urls_parsed << "\n"
        << "URLs Failed: " << stats.urls_failed << "\n";

    return 0;
}