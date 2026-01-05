#pragma once

#include "crc64.h"

namespace MessageIDS
{
    static constexpr uint64_t SetClientId = Hashes::CRC64Str("SetClientId");
    static constexpr uint64_t ServerTextMessage = Hashes::CRC64Str("ServerTextMessage");
    static constexpr uint64_t ServerAddChatUser = Hashes::CRC64Str("ServerAddChatUser");
	static constexpr uint64_t ServerRemoveChatUser = Hashes::CRC64Str("ServerRemoveChatUser");

    static constexpr uint64_t WorldInfo = Hashes::CRC64Str("WorldInfo");
}
