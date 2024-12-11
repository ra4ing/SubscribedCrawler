#include <iostream>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include "../include/logger.h"
#include "../include/config.h"


std::shared_ptr<Logger> Logger::GetLogger()
{
    static std::shared_ptr<Logger> instance = std::make_shared<Logger>();
    return instance;
}

Logger::Logger() {
    try {
        // 初始化异步日志线程池（队列大小为8192，线程数为1）
        spdlog::init_thread_pool(8192, 4);

        // 获取日志文件路径
        std::string log_file = Config::GetConfig()->getLogFileDir();

        // 创建异步文件日志记录器
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, 1048576 * 5, 3);

        // 创建异步 logger，使用线程池
        logger_ = std::make_shared<spdlog::async_logger>(
            "async_file_logger",
            file_sink,
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        // 设置日志格式
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        // 设置日志级别
        logger_->set_level(spdlog::level::info);

        logger_->flush_on(spdlog::level::err);

        // 注册 logger
        spdlog::register_logger(logger_);

        // 设置全局错误处理器
        spdlog::set_error_handler([](const std::string& msg){
            std::cerr << "Log error: " << msg << std::endl;
        });

    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "异步日志初始化失败: " << ex.what() << std::endl;
    }
}

Logger::~Logger() {
    try {
        flush();
        spdlog::shutdown();
        std::cout << "Logger shutdown" << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "Logger shutdown failed: " << ex.what() << std::endl;
    }
}

void Logger::Info(const std::string& msg)
{
    logger_->info(msg);
}

void Logger::Warn(const std::string& msg)
{
    logger_->warn(msg);
}

void Logger::Error(const std::string& msg)
{
    logger_->error(msg);
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->flush();
}