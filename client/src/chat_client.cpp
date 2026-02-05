#include "chat_client.h"

#include "connection.h"

#include "messages/chat_group_messages.h"
#include "messages/chat_messages.h"
#include "player_database.h"
#include <unordered_map>

namespace ChatClient
{
	Events::EventSource<uint32_t> OnChannelAdded;
    Events::EventSource<uint32_t> OnChannelRemoved;

	std::unordered_map<uint32_t, Group> Groups;

	uint32_t UserID = 0;

	void HandleServerAddChatUser(const ChatGroupMessages::ServerAddChatUser& message)
	{
		PlayerDatabase::User user;
		user.Name = message.UserName();

		PlayerDatabase::Users.insert_or_assign(message.UserID(), user);
	}

	void HandleServerRemoveChatUser(const ChatGroupMessages::ServerRemoveChatUser& message)
	{
		auto itr = PlayerDatabase::Users.find(message.UserID());
		if (itr != PlayerDatabase::Users.end())
			itr->second.Active = false;
	}

	void HandleServerTextMessage(const ChatMessages::ServerTextMessage& message)
	{
		Message chatMessage;
		chatMessage.SenderId = message.SenderID();
		chatMessage.Message = message.Message();

		auto  groupItr = Groups.find(message.GroupID());
        if (groupItr == Groups.end())
		{
			return;
		}
		
        groupItr->second.ChatLog.push_back(chatMessage);
	}

	void HandleServerSetChatGroup(const ChatGroupMessages::ServerSetChatGroup& message)
	{
        Group group;
        group.ID = message.GroupId();
        group.Name = message.Name();
		Groups.insert_or_assign(group.ID, std::move(group));
		OnChannelAdded.Invoke(nullptr, message.GroupId());
    }

	void Init()
	{
		Cleanup();

        Groups.insert_or_assign(0, Group{ 0, "General", {} });
		OnChannelAdded.Invoke(nullptr, 0);

		Connection::RegisterHandler<ChatGroupMessages::ServerAddChatUser>().ProcessFunc = HandleServerAddChatUser;
		Connection::RegisterHandler<ChatGroupMessages::ServerRemoveChatUser>().ProcessFunc = HandleServerRemoveChatUser;
        Connection::RegisterHandler<ChatGroupMessages::ServerSetChatGroup>().ProcessFunc = HandleServerSetChatGroup;

		Connection::RegisterHandler<ChatMessages::ServerTextMessage>().ProcessFunc = HandleServerTextMessage;
	}

	void Cleanup()
	{
		for (auto& [id, group] : Groups)
		{
            OnChannelRemoved.Invoke(nullptr, id);
		}
		Groups.clear();
    }

	void Process()
	{

	}

	Group* GetGroup(uint32_t id)
	{
        auto itr = Groups.find(id);
		if (itr == Groups.end())
			return nullptr;

        return &(itr->second);
	}

	void Send(uint32_t groupID, std::string_view message)
	{
		auto* group = GetGroup(groupID);
		if (!group)
			return;

        group->ChatLog.push_back(Message{ Connection::GetClientId(), std::string(message) });

		ChatMessages::ClientTextMessage msg(message, 0, groupID, false);
		Connection::Send(msg);
	}

	void SendDirect(uint32_t targetID, std::string_view message)
	{
        auto* group = GetGroup(0);
        if (!group)
            return;
		group->ChatLog.push_back(Message{ Connection::GetClientId(), std::string(message) });

        ChatMessages::ClientTextMessage msg(message, targetID, 0, true);
        Connection::Send(msg);
	}

	void SetUserID(uint32_t id)
	{
		UserID = id;
	}

	uint32_t GetUserID()
	{
		return UserID;
	}
}