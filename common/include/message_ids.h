#pragma once

#include "crc64.h"

namespace MessageIDS
{
    static constexpr uint64_t SetClientId = Hashes::CRC64Str("SetClientId");
    static constexpr uint64_t ServerTextMessage = Hashes::CRC64Str("ServerTextMessage");

    static constexpr uint64_t WorldInfo = Hashes::CRC64Str("WorldInfo");
}
