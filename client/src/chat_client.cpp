#include "chat_client.h"

#include "connection.h"

#include "messages/chat_group_messages.h"
#include "messages/chat_messages.h"

#include <vector>
#include <unordered_map>

namespace ChatClient
{
	std::vector<ChatMessage> ServerChat;
	std::unordered_map<uint64_t, ChatUser> ChatUsers;

	void HandleServerAddChatUser(const ChatGroupMessages::ServerAddChatUser& message)
	{
		ChatUser user;
		user.Name = message.UserName();

		ChatUsers.insert_or_assign(message.UserID(), user);
	}

	void HandleServerRemoveChatUser(const ChatGroupMessages::ServerRemoveChatUser& message)
	{
		auto itr = ChatUsers.find(message.UserID());
		if (itr != ChatUsers.end())
			itr->second.Active = false;
	}

	void HandleServerTextMessageUser(const ChatMessages::ServerTextMessage& message)
	{
		ChatMessage chatItem;
		chatItem.SenderId = message.SenderID();
		chatItem.Message = message.Message();

		// filter?

		ServerChat.push_back(chatItem);
	}

	void Init()
	{
		Connection::RegisterHandler<ChatGroupMessages::ServerAddChatUser>().ProcessFunc = HandleServerAddChatUser;
		Connection::RegisterHandler<ChatGroupMessages::ServerRemoveChatUser>().ProcessFunc = HandleServerRemoveChatUser;
		Connection::RegisterHandler<ChatMessages::ServerTextMessage>().ProcessFunc = HandleServerTextMessageUser;
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