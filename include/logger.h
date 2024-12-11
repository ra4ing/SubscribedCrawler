#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <memory>
#include <spdlog/spdlog.h>

class Logger {
public:
    Logger();
    ~Logger();

    static std::shared_ptr<Logger> GetLogger();

    void Info(const std::string& msg);
    void Warn(const std::string& msg);
    void Error(const std::string& msg);

    void flush();
private:
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::shared_ptr<spdlog::logger> logger_;
    std::mutex mutex_;
};

#endif // __LOGGER_H__