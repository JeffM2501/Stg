#pragma once

#include "CRC64.h"

namespace MessageIDS
{
    static constexpr uint64_t SetClientId = Hashes::CRC64Str("SetClientId");
}
