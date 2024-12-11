#include <iostream>
#include "../../include/thread_pool.h"
#include "../../include/logger.h"

ThreadPool::ThreadPool(size_t thread_count) : stop_(false) {
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this]() { WorkerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    if (!stop_) {
        Shutdown();
    }
}

void ThreadPool::WorkerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);

            condition_.wait(lock, [this]() { return !tasks_.empty() || stop_; });
            if (stop_) return;

            task = std::move(tasks_.front());
            tasks_.pop();
        }
        try {
            task();
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->Error(e.what());
        }
        catch (...) {
            Logger::GetLogger()->Error("Unkown error occured in threa pool.");
        }

    }
}

void ThreadPool::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }

    condition_.notify_all();
    for (std::thread& worker : workers_) {
        // if (worker.joinable())
            worker.join();
    }
    std::cout << "ThreadPool shutdown" << std::endl;
}