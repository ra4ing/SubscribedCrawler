#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/url.hpp>
#include <openssl/ssl.h>
#include <iostream>
#include <string>
#include "../../include/crawler/http_client.h"
#include "../../include/logger.h"

HttpClient::HttpClient() : stop_(false), logger_(Logger::GetLogger()) {}

HttpClient::~HttpClient() {}


void HttpClient::httpGet(
    FetchResult& fetch_result,
    const BoostReq& req,
    const std::string& host,
    const std::string& service) const
{
    try {
        boost::asio::io_context ioc;

        // 解析主机名和端口
        boost::asio::ip::tcp::resolver resolver(ioc);
        auto results = resolver.resolve(host, service);

        boost::beast::tcp_stream stream(ioc);
        stream.connect(results);

        boost::beast::http::write(stream, req);

        BoostRes res;
        boost::beast::flat_buffer buffer;
        boost::beast::http::read(stream, buffer, res);

        boost::system::error_code ec;
        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        if (ec == boost::asio::error::eof || ec == boost::asio::ssl::error::stream_truncated) {
            // 这些错误是预期的，可以安全忽略
            ec = {};
        }
        if (ec && ec != boost::asio::error::eof) {
            throw boost::system::system_error{ ec };
        }

        fetch_result.success = true;
        fetch_result.body = boost::beast::buffers_to_string(res.body().data());
        fetch_result.http_status_code = res.result_int();
    }
    catch (const boost::system::system_error& e) {
        // 处理 Boost.System 错误
        logger_->Warn(std::string("Boost.System Error: ") + e.what() + "\nURL: " + host);
    }
    catch (std::exception const& e) {
        logger_->Warn(std::string("HttpGet Error: ") + e.what() + "\nURL: " + host);
    }
    catch (...) {
        logger_->Warn(std::string("Unknown Error.") + "\nURL: " + host);
    }
}

void HttpClient::httpsGet(
    FetchResult& fetch_result,
    const BoostReq& req,
    const std::string& host,
    const std::string& service) const
{
    try {
        boost::asio::io_context ioc;

        // 初始化 SSL 上下文，使用 TLS 客户端方法
        boost::asio::ssl::context ctx(boost::asio::ssl::context::tls_client);
        ctx.set_verify_mode(boost::asio::ssl::verify_peer); // 在生产环境中建议使用 verify_peer
        ctx.set_default_verify_paths();

        // 解析主机名和端口
        boost::asio::ip::tcp::resolver resolver(ioc);
        auto results = resolver.resolve(host, service);

        // 创建 SSL 流
        boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);

        // 设置 SNI 主机名
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
            boost::beast::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
            throw boost::system::system_error{ ec };
        }

        boost::beast::get_lowest_layer(stream).connect(results);

        // Perform the SSL handshake
        stream.handshake(boost::asio::ssl::stream_base::client);

        boost::beast::http::write(stream, req);

        BoostRes res;
        boost::beast::flat_buffer buffer;
        boost::beast::http::read(stream, buffer, res);

        boost::system::error_code ec;
        stream.shutdown(ec);

        if (ec == boost::asio::error::eof || ec == boost::asio::ssl::error::stream_truncated) {
            ec = {};
        }
        if (ec && ec != boost::asio::error::eof) {
            throw boost::system::system_error{ ec };
        }
        fetch_result.success = true;
        fetch_result.body = boost::beast::buffers_to_string(res.body().data());
        fetch_result.http_status_code = res.result_int();
    }
    catch (const boost::system::system_error& e) {
        logger_->Warn(std::string("Boost.System Error: ") + e.what() + "\nURL: " + host);
    }
    catch (std::exception const& e) {
        logger_->Warn(std::string("HttpsGet Error: ") + e.what() + "\nURL: " + host);
    }
    catch (...) {
        logger_->Warn(std::string("Unknown Error.") + "\nURL: " + host);
    }
}

void HttpClient::Fetch(
    FetchResult& fetch_result,
    const std::string& url,
    const std::string& user_agent,
    const size_t timeout,
    int max_retries) const
{

    // 使用 parse_uri 来解析和验证 URL
    boost::urls::result<boost::urls::url_view> parse_result = boost::urls::parse_uri(url);
    if (!parse_result) {
        logger_->Warn("Invalid URL: " + url);
        return;
    }
    boost::urls::url_view url_view = parse_result.value();

    // 获取主机名
    std::string host = url_view.host();
    if (host.empty()) {
        logger_->Warn("URL lost host: " + url);
        return;
    }

    // 获取端口号，若未指定则根据协议设置默认端口
    std::string port = url_view.port();
    if (port.empty()) {
        std::string scheme = url_view.scheme();
        if (scheme == "https") {
            port = "443";
        }
        else if (scheme == "http") {
            port = "80";
        }
        else {
            logger_->Warn("Unsupport protocol: " + scheme);
            return;
        }
    }

    std::string target(url_view.encoded_target());
    if (target.empty()) {
        target = "/";
    }

    BoostReq req;
    BoostRes res;
    req.method(boost::beast::http::verb::get);
    req.target(target);
    req.set(boost::beast::http::field::host, host);
    req.set(boost::beast::http::field::user_agent,
        user_agent.empty() ? "Boost.Beast.HttpClient/1.0" : user_agent
    );

    for (int attempt = 0; attempt < max_retries && !stop_; ++attempt) {
        try {
            if (url_view.scheme() == "https") {
                httpsGet(fetch_result, req, host, port);
            }
            else if (url_view.scheme() == "http") {
                httpGet(fetch_result, req, host, port);
            }
            else {
                logger_->Warn("Not support protocol: " + std::string(url_view.scheme()));
            }

            if (fetch_result.success)
                return;
        }
        catch (const std::exception& e) {
            logger_->Warn(std::string("Fetch Error: ") + e.what());
        }
        catch (...) {
            logger_->Warn(std::string("Unknown Error."));
        }

        if (stop_) return;
        if (attempt < max_retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * attempt));
            logger_->Info("Retrying (" + std::to_string(attempt) + "/" + std::to_string(max_retries) + "): " + url);
        }
        else {
            logger_->Warn("Max retries reached for URL: " + url);
            return;
        }
    }
}

void HttpClient::Shutdown()
{
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
    std::cout << "HttpClient shutdown" << std::endl;
}


