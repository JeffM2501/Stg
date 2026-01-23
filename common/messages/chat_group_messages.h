#pragma once

#include "messages.h"
#include "crc64.h"

namespace MessageIDS
{
	static const uint64_t ServerAddChatUser = Hashes::CRC64Str("ServerAddChatUser");
	static const uint64_t ServerRemoveChatUser = Hashes::CRC64Str("ServerRemoveChatUser");
	static const uint64_t ServerSetChatGroup = Hashes::CRC64Str("ServerSetChatGroup");
	static const uint64_t TestMessage = Hashes::CRC64Str("TestMessage");
}

namespace ChatGroupMessages
{
	class ServerAddChatUser :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerAddChatUser);
		ServerAddChatUser(uint32_t userID, std::string_view userName, uint32_t groupID = 0) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(sizeof(uint32_t) + sizeof(uint32_t) + GetBufferWriteSize(userName.size()));

			size_t offset = 0;
			UserIDOffset = offset;
			offset += sizeof(uint32_t);
			GroupIDOffset = offset;
			offset += sizeof(uint32_t);
			UserNameOffset = offset;
			offset += GetBufferWriteSize(userName.size());

			WriteTypeID(MessageIDS::ServerAddChatUser);
			SetUserID(userID);
			SetGroupID(groupID);
			SetUserName(userName);
		}

		ServerAddChatUser(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Chat;

			size_t offset = 0;
			UserIDOffset = offset;
			offset += sizeof(uint32_t);
			GroupIDOffset = offset;
			offset += sizeof(uint32_t);
			UserNameOffset = offset;
			offset += ReadBufferSize(UserNameOffset);
		}

		int GetProcessingChannel() override { return RouteID::ChatHandler; }

		size_t UserIDOffset = 0;
		size_t GroupIDOffset = 0;
		size_t UserNameOffset = 0;

		uint32_t UserID() const { return *ReadValue<uint32_t>(UserIDOffset); }
		void SetUserID(uint32_t value) { WriteValue<uint32_t>(value, UserIDOffset); }

		uint32_t GroupID() const { return *ReadValue<uint32_t>(GroupIDOffset); }
		void SetGroupID(uint32_t value) { WriteValue<uint32_t>(value, GroupIDOffset); }

		std::string_view UserName() const { return ReadString(UserNameOffset); }
		void SetUserName(std::string_view value) { WriteBufferValue(value.data(), value.size(), UserNameOffset); }
	};

	class ServerRemoveChatUser :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerRemoveChatUser);
		ServerRemoveChatUser(uint32_t userID, uint32_t groupID = 0) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(sizeof(uint32_t) + sizeof(uint32_t));

			size_t offset = 0;
			UserIDOffset = offset;
			offset += sizeof(uint32_t);
			GroupIDOffset = offset;
			offset += sizeof(uint32_t);

			WriteTypeID(MessageIDS::ServerRemoveChatUser);
			SetUserID(userID);
			SetGroupID(groupID);
		}

		ServerRemoveChatUser(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Chat;

			size_t offset = 0;
			UserIDOffset = offset;
			offset += sizeof(uint32_t);
			GroupIDOffset = offset;
			offset += sizeof(uint32_t);
		}

		int GetProcessingChannel() override { return RouteID::ChatHandler; }

		size_t UserIDOffset = 0;
		size_t GroupIDOffset = 0;

		uint32_t UserID() const { return *ReadValue<uint32_t>(UserIDOffset); }
		void SetUserID(uint32_t value) { WriteValue<uint32_t>(value, UserIDOffset); }

		uint32_t GroupID() const { return *ReadValue<uint32_t>(GroupIDOffset); }
		void SetGroupID(uint32_t value) { WriteValue<uint32_t>(value, GroupIDOffset); }
	};

	class ServerSetChatGroup :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerSetChatGroup);
		ServerSetChatGroup(uint32_t groupId, std::string_view name, bool isPrivate = false) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(sizeof(uint32_t) + sizeof(bool) + GetBufferWriteSize(name.size()));

			size_t offset = 0;
			GroupIdOffset = offset;
			offset += sizeof(uint32_t);
			IsPrivateOffset = offset;
			offset += sizeof(bool);
			NameOffset = offset;
			offset += GetBufferWriteSize(name.size());

			WriteTypeID(MessageIDS::ServerSetChatGroup);
			SetGroupId(groupId);
			SetIsPrivate(isPrivate);
			SetName(name);
		}

		ServerSetChatGroup(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Chat;

			size_t offset = 0;
			GroupIdOffset = offset;
			offset += sizeof(uint32_t);
			IsPrivateOffset = offset;
			offset += sizeof(bool);
			NameOffset = offset;
			offset += ReadBufferSize(NameOffset);
		}

		int GetProcessingChannel() override { return RouteID::ChatHandler; }

		size_t GroupIdOffset = 0;
		size_t IsPrivateOffset = 0;
		size_t NameOffset = 0;

		uint32_t GroupId() const { return *ReadValue<uint32_t>(GroupIdOffset); }
		void SetGroupId(uint32_t value) { WriteValue<uint32_t>(value, GroupIdOffset); }

		bool IsPrivate() const { return *ReadValue<bool>(IsPrivateOffset); }
		void SetIsPrivate(bool value) { WriteValue<bool>(value, IsPrivateOffset); }

		std::string_view Name() const { return ReadString(NameOffset); }
		void SetName(std::string_view value) { WriteBufferValue(value.data(), value.size(), NameOffset); }
	};

	class TestMessage :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::TestMessage);
		TestMessage(std::string_view stringValue, uint32_t intValue = 0) : MessageBuffer(nullptr)
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

		TestMessage(ENetPacket* packet) : MessageBuffer(packet)
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

		std::string_view StringValue() const { return ReadString(StringValueOffset); }
		void SetStringValue(std::string_view value) { WriteBufferValue(value.data(), value.size(), StringValueOffset); }

		uint32_t IntValue() const { return *ReadValue<uint32_t>(IntValueOffset); }
		void SetIntValue(uint32_t value) { WriteValue<uint32_t>(value, IntValueOffset); }
	};

	inline void Register()
	{
		MessageFactories::RegisterFactory(MessageIDS::ServerAddChatUser, [](ENetPacket* packet) { return std::make_unique<ServerAddChatUser>(packet); });
		MessageFactories::RegisterFactory(MessageIDS::ServerRemoveChatUser, [](ENetPacket* packet) { return std::make_unique<ServerRemoveChatUser>(packet); });
		MessageFactories::RegisterFactory(MessageIDS::ServerSetChatGroup, [](ENetPacket* packet) { return std::make_unique<ServerSetChatGroup>(packet); });
		MessageFactories::RegisterFactory(MessageIDS::TestMessage, [](ENetPacket* packet) { return std::make_unique<TestMessage>(packet); });
	}
} //ChatGroupMessages
