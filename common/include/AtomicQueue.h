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

		return Size() == 0;
	}

	T Pop()
	{
		auto lock = GetLock();
		T val = front();
		pop_front();
		return val;
	}

	void Push(const T& val)
	{
		auto lock = GetLock();
		push_back(val);
	}
};