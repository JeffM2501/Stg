#include "chat_client.h"

#include "connection.h"

#include "chat_messages.h"

#include <vector>
#include <unordered_map>

namespace ChatClient
{
	std::vector<ChatMessage> ServerChat;
	std::unordered_map<uint64_t, ChatUser> ChatUsers;

	void HandleServerAddChatUser(const Unpack::ServerAddChatUser& message)
	{
		ChatUser user;
		user.Name = message.Name;

		ChatUsers.insert_or_assign(message.UserID, user);
	}

	void HandleServerRemoveChatUser(const Unpack::ServerRemoveChatUser& message)
	{
		auto itr = ChatUsers.find(message.UserID);
		if (itr != ChatUsers.end())
			itr->second.Active = false;
	}

	void HandleServerTextMessageUser(const Unpack::ServerTextMessage& message)
	{
		ChatMessage chatItem;
		chatItem.SenderId = message.SenderId;
		chatItem.Message = message.Message;

		// filter?

		ServerChat.push_back(chatItem);
	}

	void Init()
	{
		Connection::RegisterHandler<Unpack::ServerAddChatUser>().ProcessFunc = HandleServerAddChatUser;
		Connection::RegisterHandler<Unpack::ServerRemoveChatUser>().ProcessFunc = HandleServerRemoveChatUser;
		Connection::RegisterHandler<Unpack::ServerTextMessage>().ProcessFunc = HandleServerTextMessageUser;
	}

	void Process()
	{

	}

	ChatUser* GetUserFromId(uint64_t id)
	{
		auto itr = ChatUsers.find(id);
		if (itr != ChatUsers.end())
			return &(itr->second);

		return nullptr;
	}

	std::span<ChatMessage> GetChatLog()
	{
		return std::span<ChatMessage>(ServerChat);
	}
}