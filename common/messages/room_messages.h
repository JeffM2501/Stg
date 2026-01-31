#pragma once

#include "messages.h"
#include "crc64.h"

namespace MessageIDS
{
	static const uint64_t SetRoom = Hashes::CRC64Str("SetRoom");
	static const uint64_t AddUserToRoom = Hashes::CRC64Str("AddUserToRoom");
	static const uint64_t RemoveUserFromRoom = Hashes::CRC64Str("RemoveUserFromRoom");
}

namespace RoomMessages
{
	class SetRoom :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::SetRoom);
		SetRoom(std::string_view name, uint32_t roomId = 0, uint32_t flags = 0, bool isPrivate = false) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Control;
			AllocatePacket(sizeof(uint32_t) + sizeof(uint32_t) + sizeof(bool) + GetBufferWriteSize(name.size()));

			size_t offset = 0;
			RoomIdOffset = offset;
			offset += sizeof(uint32_t);
			FlagsOffset = offset;
			offset += sizeof(uint32_t);
			IsPrivateOffset = offset;
			offset += sizeof(bool);
			NameOffset = offset;
			offset += GetBufferWriteSize(name.size());

			WriteTypeID(MessageIDS::SetRoom);
			SetRoomId(roomId);
			SetFlags(flags);
			SetIsPrivate(isPrivate);
			SetName(name);
		}

		SetRoom(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Control;

			size_t offset = 0;
			RoomIdOffset = offset;
			offset += sizeof(uint32_t);
			FlagsOffset = offset;
			offset += sizeof(uint32_t);
			IsPrivateOffset = offset;
			offset += sizeof(bool);
			NameOffset = offset;
			offset += ReadBufferSize(NameOffset);
		}

		int GetProcessingChannel() override { return RouteID::ControllHandler; }

		size_t RoomIdOffset = 0;
		size_t FlagsOffset = 0;
		size_t IsPrivateOffset = 0;
		size_t NameOffset = 0;

		uint32_t RoomId() const { return *ReadValue<uint32_t>(RoomIdOffset); }
		void SetRoomId(uint32_t value) { WriteValue<uint32_t>(value, RoomIdOffset); }

		uint32_t Flags() const { return *ReadValue<uint32_t>(FlagsOffset); }
		void SetFlags(uint32_t value) { WriteValue<uint32_t>(value, FlagsOffset); }

		bool IsPrivate() const { return *ReadValue<bool>(IsPrivateOffset); }
		void SetIsPrivate(bool value) { WriteValue<bool>(value, IsPrivateOffset); }

		std::string_view Name() const { return ReadString(NameOffset); }
		void SetName(std::string_view value) { WriteBufferValue(value.data(), value.size(), NameOffset); }
	};

	class AddUserToRoom :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::AddUserToRoom);
		AddUserToRoom(uint32_t userId = 0) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Control;
			AllocatePacket(sizeof(uint32_t));

			size_t offset = 0;
			UserIdOffset = offset;
			offset += sizeof(uint32_t);

			WriteTypeID(MessageIDS::AddUserToRoom);
			SetUserId(userId);
		}

		AddUserToRoom(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Control;

			size_t offset = 0;
			UserIdOffset = offset;
			offset += sizeof(uint32_t);
		}

		int GetProcessingChannel() override { return RouteID::ControllHandler; }

		size_t UserIdOffset = 0;

		uint32_t UserId() const { return *ReadValue<uint32_t>(UserIdOffset); }
		void SetUserId(uint32_t value) { WriteValue<uint32_t>(value, UserIdOffset); }
	};

	class RemoveUserFromRoom :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::RemoveUserFromRoom);
		RemoveUserFromRoom(uint32_t userId = 0) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Control;
			AllocatePacket(sizeof(uint32_t));

			size_t offset = 0;
			UserIdOffset = offset;
			offset += sizeof(uint32_t);

			WriteTypeID(MessageIDS::RemoveUserFromRoom);
			SetUserId(userId);
		}

		RemoveUserFromRoom(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Control;

			size_t offset = 0;
			UserIdOffset = offset;
			offset += sizeof(uint32_t);
		}

		int GetProcessingChannel() override { return RouteID::ControllHandler; }

		size_t UserIdOffset = 0;

		uint32_t UserId() const { return *ReadValue<uint32_t>(UserIdOffset); }
		void SetUserId(uint32_t value) { WriteValue<uint32_t>(value, UserIdOffset); }
	};

	inline void Register()
	{
		MessageFactories::RegisterFactory(MessageIDS::SetRoom, [](ENetPacket* packet) { return std::make_unique<SetRoom>(packet); });
		MessageFactories::RegisterFactory(MessageIDS::AddUserToRoom, [](ENetPacket* packet) { return std::make_unique<AddUserToRoom>(packet); });
		MessageFactories::RegisterFactory(MessageIDS::RemoveUserFromRoom, [](ENetPacket* packet) { return std::make_unique<RemoveUserFromRoom>(packet); });
	}
} //RoomMessages
