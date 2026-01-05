#pragma once

#include "messages.h"
#include "message_ids.h"

namespace Pack
{
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
    class ServerTextMessage : public MessageUnpackBuffer
    {
    public:
        DECLARE_MESSAGE_ID(MessageIDS::ServerTextMessage);

        uint32_t SenderId = 0;
        std::string_view Message;

        ServerTextMessage(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
            Read(SenderId);
            Read(Message);
        }
    };
}