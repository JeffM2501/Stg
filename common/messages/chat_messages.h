#pragma once

#include "messages.h"
#include "message_ids.h"

namespace ChatMessages
{
	class ServerTextMessage :public  MessagePackBuffer, MessageUnpackBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::ServerTextMessage);
		ServerTextMessage(std::string_view message, uint32_t senderID = 0, uint32_t groupID = 0, bool isPrivate = false) : MessageUnpackBuffer(nullptr)
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

		ServerTextMessage(ENetPacket* packet) : MessageUnpackBuffer(packet)
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

} //ChatMessages
