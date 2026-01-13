#pragma once

#include <string_view>

struct ConnectedClient;

namespace ChatSystem
{
	void Init();
	void Process();

	uint32_t CreateChatGroup(std::string_view groupName);
	void DestroyChatGroup(uint32_t groupID);

	void AddUserToGroup(ConnectedClient* client, uint32_t groupID);
	void RemoveUserFromGroup(ConnectedClient* client, uint32_t groupID);

	void SendServerMessage(std::string_view message, ConnectedClient* target = nullptr);
	void SendServerMessage(std::string_view message, uint32_t channelID);
}