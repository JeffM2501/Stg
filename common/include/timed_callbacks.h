#pragma once

#include <unordered_map>
#include <functional>
#include <string_view>

class TimedCallbackHost
{
private:
    struct CallbackEntry
    {
        float Durration = 0.0f;
        float Accumulator = 0.0f;

        bool Repeat = false;

        std::function<void(size_t)> Callback;
    };
    std::unordered_map<size_t, CallbackEntry> Callbacks;

public:
    void Update(float dt);

    size_t Add(std::string_view name, float durration, std::function<void(size_t)> callback, bool repeat = false);
    void Remove(size_t id);
};