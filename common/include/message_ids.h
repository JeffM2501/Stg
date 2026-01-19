#pragma once

#include "crc64.h"

namespace MessageIDS
{
    static constexpr uint64_t SendClientId = Hashes::CRC64Str("SendClientId");
    static constexpr uint64_t ServerTextMessage = Hashes::CRC64Str("ServerTextMessage");
    static constexpr uint64_t ServerAddChatUser = Hashes::CRC64Str("ServerAddChatUser");
	static constexpr uint64_t ServerRemoveChatUser = Hashes::CRC64Str("ServerRemoveChatUser");
	static constexpr uint64_t ServerSetChatGroup = Hashes::CRC64Str("ServerSetChatGroup");

    static constexpr uint64_t WorldInfo = Hashes::CRC64Str("WorldInfo"); 
    
    static constexpr uint64_t TestMessage = Hashes::CRC64Str("TestMessage");
}
