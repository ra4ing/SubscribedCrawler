#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <boost/beast.hpp>
#include <chrono>
#include <string>
#include <optional>
#include "../../include/logger.h"
#include "../common_types.h"

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    // Fetch a URL with a given timeout and user-agent.
    void Fetch(
        FetchResult& fetch_result,
        const std::string& url,
        const std::string& user_agent,
        const size_t timeout,
        int max_retries = 2) const;

private:
    std::shared_ptr<Logger> logger_;
    std::mutex mutex_;
    bool stop_;

    using BoostReq = boost::beast::http::request<boost::beast::http::string_body>;
    using BoostRes = boost::beast::http::response<boost::beast::http::dynamic_body>;
    void httpGet(
        FetchResult& fetch_result,
        const BoostReq& req,
        const std::string& host, 
        const std::string& service) const;
    void httpsGet(
        FetchResult& fetch_result,
        const BoostReq& req, 
        const std::string& host, 
        const std::string& service) const;
    void Shutdown();
};

#endif // __HTTP_CLIENT_H__