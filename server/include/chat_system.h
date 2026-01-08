#pragma once

#include <string_view>

struct ConnectedClient;

namespace ChatSystem
{
	void Init();
	void Process();

	size_t CreateChatGroup(std::string_view groupName);
	void DestroyChatGroup(size_t groupID);

	void AddUserToGroup(ConnectedClient* client, size_t groupID);
	void RemoveUserFromGroup(ConnectedClient* client, size_t groupID);

	void SendServerMessage(std::string_view message, ConnectedClient* target = nullptr);
	void SendServerMessage(std::string_view message, size_t channelID);
}