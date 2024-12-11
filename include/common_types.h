#ifndef __COMMON_TYPES_H__
#define __COMMON_TYPES_H__

#include <string>
#include <vector>
#include <optional>
#include <chrono>


struct FetchResult {
    bool success;
    std::string body;
    unsigned int http_status_code;
};

struct ParsedLinks {
    std::vector<std::string> links;
};

enum class CrawlerErrorCode {
    kOk = 0,
    kNetworkError,
    kParseError,
    kInvalidUrl,
    kTimeout,
    kOther,
};

struct CrawlerStats {
    int urls_fetched;
    int urls_parsed;
    int urls_failed;
};



#endif // __COMMON_TYPES_H__