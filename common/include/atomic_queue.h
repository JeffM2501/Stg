#pragma once

#include <mutex>
#include <deque>

template <class T>
class AtomicQueue : public std::deque<T>
{
private:
    std::mutex Mutex;

public:
    std::lock_guard<std::mutex> GetLock()
    {
        return std::lock_guard<std::mutex>(Mutex);
    }

    bool Empty()
    {
        auto lock = GetLock();

        return this->size() == 0;
    }

    T Pop()
    {
        auto lock = GetLock();
        T val = std::move(this->front());
        this->pop_front();
        return val;
    }

    void Push(T& val)
    {
        auto lock = GetLock();
        this->emplace_back(std::move(val));
    }
};