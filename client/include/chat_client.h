#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace ChatClient
{
	struct ChatMessage
	{
		uint64_t SenderId;
		std::string Message;
	};

	struct ChatUser
	{
		std::string Name;
		uint32_t RoomId = uint32_t(-1);
		bool Active = true;
	};

	void Init();
	void Process();

	ChatUser* GetUserFromId(uint64_t id);

	std::span<ChatMessage> GetChatLog();
}