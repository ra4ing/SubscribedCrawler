#ifndef __URL_QUEUE_H__
#define __URL_QUEUE_H__

#include <optional>
#include <condition_variable>
#include <queue>

class UrlQueue
{
public:
    UrlQueue();
    ~UrlQueue();

    void Push(const std::string& url);

    std::optional<std::string> Pop();

    bool Empty() const;
    void Shutdown();

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::string> queue_;
    bool stop_;
};


#endif // __URL_QUEUE_H__