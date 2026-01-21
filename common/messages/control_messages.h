#pragma once

#include "messages.h"
#include "message_ids.h"

namespace ControlMessages
{
	class SendClientId :public MessageBuffer
	{
	public:
		DECLARE_MESSAGE_ID(MessageIDS::SendClientId);
		SendClientId(uint32_t clientID = 0) : MessageBuffer(nullptr)
		{
			Channel = NetworkChannelIDs::Control;
			AllocatePacket(sizeof(uint32_t));

			size_t offset = 0;
			ClientIDOffset = offset;
			offset += sizeof(uint32_t);

			WriteTypeID(MessageIDS::SendClientId);
			SetClientID(clientID);
		}

		SendClientId(ENetPacket* packet) : MessageBuffer(packet)
		{
			Channel = NetworkChannelIDs::Control;

			size_t offset = 0;
			ClientIDOffset = offset;
			offset += sizeof(uint32_t);
		}

		int GetProcessingChannel() override { return RouteID::ControllHandler; }

		size_t ClientIDOffset = 0;

		uint32_t ClientID() const { return *ReadValue<uint32_t>(ClientIDOffset); }
		void SetClientID(uint32_t value) { WriteValue<uint32_t>(value, ClientIDOffset); }
	};

} //ControlMessages
