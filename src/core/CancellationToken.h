#pragma once
#include <atomic>

class CancellationToken {
public:
    void cancel()             { flag_.store(true,  std::memory_order_relaxed); }
    bool isCancelled() const  { return flag_.load(std::memory_order_relaxed); }
    void reset()              { flag_.store(false, std::memory_order_relaxed); }
private:
    std::atomic<bool> flag_{false};
};
