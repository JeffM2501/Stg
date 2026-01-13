#pragma once

#include "messages.h"
#include "message_ids.h"

namespace ChatGroupMessages
{
	class ServerAddChatUser : public MessagePackBuffer, MessageUnpackBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerAddChatUser);
		ServerAddChatUser(uint32_t userId, std::string_view userName, uint32_t groupId = 0) : MessageUnpackBuffer(nullptr)
		{
            Channel = NetworkChannelIDs::Chat;
            AllocatePacket(GetBufferWriteSize(userName.size()) + sizeof(uint32_t) + sizeof(uint32_t));
            WriteTypeID(MessageIDS::ServerAddChatUser);
            SetUserID(userId);
            SetUserName(userName);
		}

		ServerAddChatUser(ENetPacket* packet) : MessageUnpackBuffer(packet)
		{
            Channel = NetworkChannelIDs::Chat;
		}

        int GetProcessingChannel() override { return RouteID::ChatHandler; }

		static constexpr size_t UserIDOffset = 0;
        static constexpr size_t GroupIDOffset = 4;
        static constexpr size_t UserNameOffset = 8;

		uint32_t UserID() const { return *ReadValue<uint32_t>(UserIDOffset); }
        void SetUserID(uint32_t id) { WriteValue<uint32_t>(id, UserIDOffset); }

		uint32_t GroupID() const { return *ReadValue<uint32_t>(GroupIDOffset); }
        void SetGroupID(uint32_t id)  { WriteValue<uint32_t>(id, GroupIDOffset); }

		std::string_view UserName() const { return ReadString(UserNameOffset); }
        void SetUserName(std::string_view userName) { WriteBufferValue(userName.data(), userName.size(), UserNameOffset);  }
	};

    class ServerRemoveChatUser : public MessagePackBuffer, MessageUnpackBuffer
    {
    public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerRemoveChatUser);
        ServerRemoveChatUser(uint32_t userId, uint32_t groupId = 0) : MessageUnpackBuffer(nullptr)
        {
            Channel = NetworkChannelIDs::Chat;
            AllocatePacket(sizeof(uint32_t) + sizeof(uint32_t));
            WriteTypeID(MessageIDS::ServerRemoveChatUser);
            SetUserId(userId);
			SetGroupID(groupId);
        }

		ServerRemoveChatUser(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
            Channel = NetworkChannelIDs::Chat;
        }

        static constexpr size_t UserIDOffset = 0;
        static constexpr size_t GroupIDOffset = 4;

		uint32_t UserID() const { return *ReadValue<uint32_t>(UserIDOffset); }
        void SetUserId(uint32_t id) { WriteValue<uint32_t>(id, UserIDOffset); }

        uint32_t GroupID() const { return *ReadValue<uint32_t>(GroupIDOffset); }
        void SetGroupID(uint32_t id) { WriteValue<uint32_t>(id, GroupIDOffset); }
    };

    class ServerSetChatGroup : public MessagePackBuffer, MessageUnpackBuffer
    {
    public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerSetChatGroup);
		ServerSetChatGroup(uint32_t groupId, std::string_view name, uint8_t isPrivate = 0) : MessageUnpackBuffer(nullptr)
        {
            Channel = NetworkChannelIDs::Chat;
			AllocatePacket(GetBufferWriteSize(name.size()) + sizeof(uint32_t) + sizeof(uint8_t));
            WriteTypeID(MessageIDS::ServerSetChatGroup);
			SetGroupID(groupId);
        }

		ServerSetChatGroup(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
            Channel = NetworkChannelIDs::Chat;
        }

        static constexpr size_t GroupIDOffset = 0;
		static constexpr size_t IsPrivateOffset = 4;
        static constexpr size_t NameOffset = 5;

        uint32_t GroupID() const { return *ReadValue<uint32_t>(GroupIDOffset); }
        void SetGroupID(uint32_t id) { WriteValue<uint32_t>(id, GroupIDOffset); }

		uint8_t IsPrivate() const { return *ReadValue<uint8_t>(IsPrivateOffset); }
        void SetIsPrivate(uint8_t isPrivate) { WriteValue<uint8_t>(isPrivate, IsPrivateOffset); }

        std::string_view Name() const { return ReadString(NameOffset); }
        void SetName(std::string_view name) { WriteBufferValue(name.data(), name.size(), NameOffset); }
    };

    class TestMessage : public MessagePackBuffer, MessageUnpackBuffer
    {
    public:
        DECLARE_MESSAGE_ID(MessageIDS::TestMessage);
		TestMessage(std::string_view stringValue, uint32_t intValue = 0) : MessageUnpackBuffer(nullptr)
        {
            Channel = NetworkChannelIDs::Chat;
            AllocatePacket(GetBufferWriteSize(stringValue.size()) + sizeof(uint32_t));

            size_t offset = 0;

            StringValueOffset = offset;
			offset += GetBufferWriteSize(stringValue.size());
            IntValueOffset = offset;
            offset += sizeof(uint32_t);

            WriteTypeID(MessageIDS::TestMessage);
            SetStringValue(stringValue);
            SetIntValue(intValue);
        }

		TestMessage(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
            Channel = NetworkChannelIDs::Chat;

			size_t offset = 0;

			StringValueOffset = offset;
			offset += ReadBufferSize(StringValueOffset);
			IntValueOffset = offset;
			offset += sizeof(uint32_t);
        }

        int GetProcessingChannel() override { return RouteID::ChatHandler; }

		size_t StringValueOffset = 0;
		size_t IntValueOffset = 0;

		std::string_view  StringValue() const { return ReadString(StringValueOffset); }
		void SetStringValue(std::string_view value) { WriteBufferValue(value.data(), value.size(), StringValueOffset); }

        uint32_t IntValue() const { return *ReadValue<uint32_t>(IntValueOffset); }
        void SetIntValue(uint32_t id) { WriteValue<uint32_t>(id, IntValueOffset); }
    };
}

namespace Pack
{
    class ServerAddChatUser : public MessagePackBuffer
    {
	public:
		ServerAddChatUser(uint32_t userId, std::string_view userName, uint32_t groupId = 0)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(GetBufferWriteSize(userName.size()) + sizeof(uint32_t) + sizeof(uint32_t));
			WriteTypeID(MessageIDS::ServerAddChatUser);
			SetUserId(userId);
			SetUserName(userName);
		}

		void SetUserId(uint32_t id)
		{
			WriteValue<uint32_t>(id, 0);
		}

		void SetGroupId(uint32_t id)
		{
			WriteValue<uint32_t>(id, 4);
		}

		void SetUserName(std::string_view userName)
		{
			WriteBufferValue(userName.data(), userName.size(), 8);
		}
    };

	class ServerRemoveChatUser : public MessagePackBuffer
	{
	public:
		ServerRemoveChatUser(uint32_t userId)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(sizeof(uint64_t));
			WriteTypeID(MessageIDS::ServerRemoveChatUser);
			SetUserId(userId);
		}

		void SetUserId(uint32_t id)
		{
			WriteValue<uint32_t>(id, 0);
		}
	};

    class ServerTextMessage : public MessagePackBuffer
    {
    public:
        ServerTextMessage(std::string_view message, uint32_t senderId = 0, uint32_t groupID = 0, bool isPrivate = false)
        {
            Channel = NetworkChannelIDs::Chat;
            AllocatePacket(GetBufferWriteSize(message.size()) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t));
            WriteTypeID(MessageIDS::ServerTextMessage);

            SetSenderId(senderId);
			SetGroupId(groupID);
            SetMessage(message);
			SetIsPrivate(isPrivate);
        }

		void SetSenderId(uint32_t id)
		{
			WriteValue<uint32_t>(id, 0);
		}

		void SetIsPrivate(bool isPrivate)
		{
			WriteValue<uint8_t>(isPrivate ? 1 : 0, 4);
		}

		void SetGroupId(uint32_t groupID)
		{
			WriteValue<uint32_t>(groupID, 8);
		}

        void SetMessage(std::string_view message)
        {
            WriteBufferValue(message.data(), message.size(), 9);
        }
    };

	class SetChatGroupInfo : public MessagePackBuffer
	{
	public:
		SetChatGroupInfo(uint32_t groupId, std::string_view name, bool isPrivate = false)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(GetBufferWriteSize(name.size()) + sizeof(uint32_t) + sizeof(uint8_t));
			WriteTypeID(MessageIDS::ServerSetChatGroup);
			SetGroupId(groupId);
			SetIsPrivate(isPrivate);
			SetName(name);
		}

		void SetGroupId(uint32_t id)
		{
			WriteValue<uint32_t>(id, 0);
		}

		void SetIsPrivate(bool isPrivate)
		{
			WriteValue<uint8_t>(isPrivate ? 1 : 0, 4);
		}

		void SetName(std::string_view name)
		{
			WriteBufferValue(name.data(), name.size(), 5);
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

		uint32_t UserID() { return *ReadValue<uint32_t>(0); }
		uint32_t GroupID(){ return *ReadValue<uint32_t>(4); }
		std::string_view Name() { return ReadString(8); }

        ServerAddChatUser(ENetPacket* packet) : MessageUnpackBuffer(packet)
		{
		}
	};

	class ServerRemoveChatUser : public MessageUnpackBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerRemoveChatUser);

		int GetProcessingChannel() override { return RouteID::ChatHandler; }

		uint64_t UserID() { return *ReadValue<uint64_t>(0); }

		ServerRemoveChatUser(ENetPacket* packet) : MessageUnpackBuffer(packet)
		{
		}
	};

    class ServerTextMessage : public MessageUnpackBuffer
    {
    public:
        DECLARE_MESSAGE_ID(MessageIDS::ServerTextMessage);

        int GetProcessingChannel() override { return RouteID::ChatHandler; }

        uint32_t SenderId() const { return *ReadValue<uint32_t>(0); }
		uint32_t GroupId() const { return *ReadValue<uint32_t>(4); }
        std::string_view Message() const { return ReadString(9); }
		bool IsPrivate() const { return *ReadValue<uint64_t>(8) != 0; }

        ServerTextMessage(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
        }
    };  
	
	class SetChatGroupInfo : public MessageUnpackBuffer
    {
    public:
        DECLARE_MESSAGE_ID(MessageIDS::ServerSetChatGroup);

        int GetProcessingChannel() override { return RouteID::ChatHandler; }

        uint64_t GroupId = 0;
        std::string_view Name;
        bool IsPrivate = false;

		SetChatGroupInfo(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
            Read(GroupId);

            uint8_t temp = 0;
            Read(temp);
            IsPrivate = temp != 0;

            Read(Name);
        }
    };
}