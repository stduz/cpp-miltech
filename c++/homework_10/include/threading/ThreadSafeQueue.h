#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
public:
    void push(T val) {
        std::lock_guard<std::mutex> lk(mu_);
        q_.push(std::move(val));
        cv_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lk(mu_);
        cv_.wait(lk, [this]{ return !q_.empty(); });
        T val = std::move(q_.front());
        q_.pop();
        return val;
    }

    bool try_pop(T& val) {
        std::lock_guard<std::mutex> lk(mu_);
        if (q_.empty()) return false;
        val = std::move(q_.front());
        q_.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mu_);
        return q_.empty();
    }

private:
    std::queue<T> q_;
    mutable std::mutex mu_;
    std::condition_variable cv_;
};
