#pragma once

#include "messages.h"
#include "message_ids.h"

namespace Pack
{
    class ServerAddChatUser : public MessagePackBuffer
    {
	public:
		ServerAddChatUser(uint64_t userId, std::string_view userName)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(GetBufferWriteSize(userName.size()) + sizeof(uint64_t));
			WriteTypeID(MessageIDS::ServerAddChatUser);
			SetUserId(userId);
			SetUserName(userName);
		}

		void SetUserId(uint64_t id)
		{
			WriteValue<uint64_t>(id, 0);
		}
    };

	class ServerRemoveChatUser : public MessagePackBuffer
	{
	public:
		ServerRemoveChatUser(uint64_t userId)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(sizeof(uint64_t));
			WriteTypeID(MessageIDS::ServerRemoveChatUser);
			SetUserId(userId);
		}

		void SetUserId(uint64_t id)
		{
			WriteValue<uint64_t>(id, 0);
		}
	};

    class ServerTextMessage : public MessagePackBuffer
    {
    public:
        ServerTextMessage(std::string_view message, uint32_t senderId = 0)
        {
            Channel = NetworkChannelIDs::Chat;
            AllocatePacket(GetBufferWriteSize(message.size()) + sizeof(uint32_t));
            WriteTypeID(MessageIDS::ServerTextMessage);

            SetSenderId(senderId);
            SetMessage(message);
        }

        void SetSenderId(uint32_t id)
        {
            WriteValue<uint32_t>(id, 0);
        }

    private:
        void SetMessage(std::string_view message)
        {
            WriteBufferValue(message.data(), message.size(), 4);
        }
    };
}

namespace Unpack
{
	class ServerAddChatUser : public MessageUnpackBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerAddChatUser);

		int GetProcessingChannel() override { return RouteID::ChatHandler; }

        uint64_t UserID = 0;
		std::string_view Name;

        ServerAddChatUser(ENetPacket* packet) : MessageUnpackBuffer(packet)
		{
			Read(UserID);
			Read(Name);
		}
	};

	class ServerRemoveChatUser : public MessageUnpackBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerRemoveChatUser);

		int GetProcessingChannel() override { return RouteID::ChatHandler; }

		uint64_t UserID = 0;

		ServerRemoveChatUser(ENetPacket* packet) : MessageUnpackBuffer(packet)
		{
			Read(UserID);
		}
	};

    class ServerTextMessage : public MessageUnpackBuffer
    {
    public:
        DECLARE_MESSAGE_ID(MessageIDS::ServerTextMessage);

        int GetProcessingChannel() override { return RouteID::ChatHandler; }

        uint32_t SenderId = 0;
        std::string_view Message;

        ServerTextMessage(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
            Read(SenderId);
            Read(Message);
        }
    };
}