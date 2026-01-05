#include "timed_callbacks.h"

void TimedCallbackHost::Update(float dt)
{
    std::vector<size_t> toRemove;
    for (auto& [id, entry] : Callbacks)
    {
        entry.Accumulator += dt;
        while (entry.Accumulator >= entry.Durration)
        {
            entry.Accumulator -= entry.Durration;

            entry.Callback(id);

            if (!entry.Repeat)
            {
                toRemove.push_back(id);
            }
        }
    }

    for (size_t id : toRemove)
    {
        Callbacks.erase(id);
    }
}

size_t TimedCallbackHost::Add(std::string_view name, float durration, std::function<void(size_t)> callback, bool repeat)
{
    if (!callback)
        return std::numeric_limits<size_t>::max();

    size_t id = std::hash<std::string_view>()(name);

    CallbackEntry entry;
    entry.Durration = durration;
    entry.Repeat = repeat;
    entry.Callback = callback;
    Callbacks.insert_or_assign(id, std::move(entry));
    return id;
}

void TimedCallbackHost::Remove(size_t id)
{
    Callbacks.erase(id);
}