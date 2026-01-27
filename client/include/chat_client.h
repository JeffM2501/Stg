#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "Events.h"

namespace ChatClient
{
	struct Message
	{
		uint32_t SenderId;
		std::string Message;
	};

	struct Group
	{
		uint32_t ID;
		std::string Name;
		std::vector<Message> ChatLog;
	};

	struct User
	{
		std::string Name;
		bool Active = true;
	};

	void Init();
	void Cleanup();
	void Process();

    void SetUserID(uint32_t id);
    uint32_t GetUserID();

	User* GetUserFromId(uint32_t id);

	Group* GetGroup(uint32_t id);

    extern Events::EventSource<uint32_t> OnChannelAdded;
	extern Events::EventSource<uint32_t> OnChannelRemoved;

    void Send(uint32_t groupID, std::string_view message);
	void SendDirect(uint32_t targetID, std::string_view message);
}