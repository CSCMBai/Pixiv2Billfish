#include "thread_pool.h"

namespace pixiv2billfish {

ThreadPool::ThreadPool(size_t num_threads) 
    : stop_(false), active_tasks_(0) {
    
    workers_.reserve(num_threads);
    
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            condition_.wait(lock, [this] {
                return stop_ || !tasks_.empty();
            });
            
            if (stop_ && tasks_.empty()) {
                return;
            }
            
            task = std::move(tasks_.front());
            tasks_.pop();
            
            ++active_tasks_;
        }
        
        try {
            task();
        } catch (...) {
            // 捕获异常，防止线程崩溃
        }
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            --active_tasks_;
            wait_condition_.notify_all();
        }
    }
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    wait_condition_.wait(lock, [this] {
        return tasks_.empty() && active_tasks_ == 0;
    });
}

size_t ThreadPool::active_threads() const {
    return active_tasks_.load();
}

size_t ThreadPool::pending_tasks() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    
    condition_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

} // namespace pixiv2billfish
