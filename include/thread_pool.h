#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <functional>
#include <vector>
#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

class ThreadPool {
public:
    explicit ThreadPool(size_t thread_count);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <class F, class... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
        using return_type = typename std::invoke_result_t<F, Args...>;
        auto t_bind = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task = std::make_shared<std::packaged_task<return_type()>>(t_bind);
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_) return task->get_future();

            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return task->get_future();
    }
    void Shutdown();

private:
    void WorkerLoop();
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_;
};
#endif // __THREAD_POOL_H__