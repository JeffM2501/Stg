#pragma once
#include <cstdint>

enum class NetworkChannelIDs : uint8_t
{
    Control = 0,
    Chat = 1,
    Assets = 2,
    Updates = 3,
    Count = 4
};