#pragma once

#include "messages.h"
#include "crc64.h"

namespace MessageIDS
{
	static const uint64_t ServerTextMessage = Hashes::CRC64Str("ServerTextMessage");
	static const uint64_t ClientTextMessage = Hashes::CRC64Str("ClientTextMessage");
}

namespace ChatMessages
{
	class ServerTextMessage :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerTextMessage);
		ServerTextMessage(std::string_view message, uint32_t senderID = 0, uint32_t groupID = 0, bool isPrivate = false) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(sizeof(uint32_t) + sizeof(uint32_t) + sizeof(bool) + GetBufferWriteSize(message.size()));

			size_t offset = 0;
			SenderIDOffset = offset;
			offset += sizeof(uint32_t);
			GroupIDOffset = offset;
			offset += sizeof(uint32_t);
			IsPrivateOffset = offset;
			offset += sizeof(bool);
			MessageOffset = offset;
			offset += GetBufferWriteSize(message.size());

			WriteTypeID(MessageIDS::ServerTextMessage);
			SetSenderID(senderID);
			SetGroupID(groupID);
			SetIsPrivate(isPrivate);
			SetMessage(message);
		}

		ServerTextMessage(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Chat;

			size_t offset = 0;
			SenderIDOffset = offset;
			offset += sizeof(uint32_t);
			GroupIDOffset = offset;
			offset += sizeof(uint32_t);
			IsPrivateOffset = offset;
			offset += sizeof(bool);
			MessageOffset = offset;
			offset += ReadBufferSize(MessageOffset);
		}

		int GetProcessingChannel() override { return RouteID::ChatHandler; }

		size_t SenderIDOffset = 0;
		size_t GroupIDOffset = 0;
		size_t IsPrivateOffset = 0;
		size_t MessageOffset = 0;

		uint32_t SenderID() const { return *ReadValue<uint32_t>(SenderIDOffset); }
		void SetSenderID(uint32_t value) { WriteValue<uint32_t>(value, SenderIDOffset); }

		uint32_t GroupID() const { return *ReadValue<uint32_t>(GroupIDOffset); }
		void SetGroupID(uint32_t value) { WriteValue<uint32_t>(value, GroupIDOffset); }

		bool IsPrivate() const { return *ReadValue<bool>(IsPrivateOffset); }
		void SetIsPrivate(bool value) { WriteValue<bool>(value, IsPrivateOffset); }

		std::string_view Message() const { return ReadString(MessageOffset); }
		void SetMessage(std::string_view value) { WriteBufferValue(value.data(), value.size(), MessageOffset); }
	};

	class ClientTextMessage :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ClientTextMessage);
		ClientTextMessage(std::string_view message, uint32_t targetId = 0, uint32_t groupID = 0, bool isPrivate = false) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Chat;
			AllocatePacket(sizeof(uint32_t) + sizeof(uint32_t) + sizeof(bool) + GetBufferWriteSize(message.size()));

			size_t offset = 0;
			TargetIdOffset = offset;
			offset += sizeof(uint32_t);
			GroupIDOffset = offset;
			offset += sizeof(uint32_t);
			IsPrivateOffset = offset;
			offset += sizeof(bool);
			MessageOffset = offset;
			offset += GetBufferWriteSize(message.size());

			WriteTypeID(MessageIDS::ClientTextMessage);
			SetTargetId(targetId);
			SetGroupID(groupID);
			SetIsPrivate(isPrivate);
			SetMessage(message);
		}

		ClientTextMessage(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Chat;

			size_t offset = 0;
			TargetIdOffset = offset;
			offset += sizeof(uint32_t);
			GroupIDOffset = offset;
			offset += sizeof(uint32_t);
			IsPrivateOffset = offset;
			offset += sizeof(bool);
			MessageOffset = offset;
			offset += ReadBufferSize(MessageOffset);
		}

		int GetProcessingChannel() override { return RouteID::ChatHandler; }

		size_t TargetIdOffset = 0;
		size_t GroupIDOffset = 0;
		size_t IsPrivateOffset = 0;
		size_t MessageOffset = 0;

		uint32_t TargetId() const { return *ReadValue<uint32_t>(TargetIdOffset); }
		void SetTargetId(uint32_t value) { WriteValue<uint32_t>(value, TargetIdOffset); }

		uint32_t GroupID() const { return *ReadValue<uint32_t>(GroupIDOffset); }
		void SetGroupID(uint32_t value) { WriteValue<uint32_t>(value, GroupIDOffset); }

		bool IsPrivate() const { return *ReadValue<bool>(IsPrivateOffset); }
		void SetIsPrivate(bool value) { WriteValue<bool>(value, IsPrivateOffset); }

		std::string_view Message() const { return ReadString(MessageOffset); }
		void SetMessage(std::string_view value) { WriteBufferValue(value.data(), value.size(), MessageOffset); }
	};

} //ChatMessages
