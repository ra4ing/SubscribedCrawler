#include "../../include/crawler/url_queue.h"
#include <iostream>

UrlQueue::UrlQueue() : stop_(false) {}

UrlQueue::~UrlQueue() {
    if (!stop_) Shutdown();
}

void UrlQueue::Push(const std::string& url) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stop_) return;
        
        queue_.push(url);
    }
    condition_.notify_one();
}

std::optional<std::string> UrlQueue::Pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    condition_.wait(lock, [this]() { return !queue_.empty() || stop_; });
    if (stop_) return std::nullopt;

    std::string url = std::move(queue_.front());
    queue_.pop();
    return url;
}

bool UrlQueue::Empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

void UrlQueue::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_ = {};
        stop_ = true;
    }
    condition_.notify_all();
    std::cout << "UrlQueue shutdown" << std::endl;
}
