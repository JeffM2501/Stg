#pragma once

#include "messages.h"
#include "message_ids.h"

namespace ControlMessages
{
    class SendClientId :public  MessageBuffer
    {
    public:
        DECLARE_MESSAGE_ID(MessageIDS::SetClientId);
        SendClientId(uint32_t clientId) : MessageBuffer(nullptr)
        {
            Channel = NetworkChannelIDs::Control;
            AllocatePacket(sizeof(uint32_t));

            size_t offset = 0;
            ClientIDOffset = offset;
            offset += sizeof(uint32_t);

            WriteTypeID(MessageIDS::SetClientId);
            SetClientID(clientId);
        }

        SendClientId(ENetPacket* packet) : MessageBuffer(packet)
        {
            Channel = NetworkChannelIDs::Chat;

            size_t offset = 0;
            ClientIDOffset = offset;
            offset += sizeof(uint32_t);
        }

        int GetProcessingChannel() override { return RouteID::ControllHandler; }

        size_t ClientIDOffset = 0;

        uint32_t ClientID() const { return *ReadValue<uint32_t>(ClientIDOffset); }
        void SetClientID(uint32_t value) { WriteValue<uint32_t>(value, ClientIDOffset); }
    };
}

namespace Pack
{
    class WorldInfo : public MessagePackBuffer
    {
    public:
        WorldInfo(float x, float y)
        {
            Channel = NetworkChannelIDs::Control;
            AllocatePacket(sizeof(float) * 2);
            WriteTypeID(MessageIDS::WorldInfo);
            SetWorldSize(x, y);
        }

        void SetWorldSize(float x, float y)
        {
            WriteValue<float>(x, 0);
            WriteValue<float>(y, sizeof(float));
        }
    };
}

namespace Unpack
{
    class SendClientId : public MessageUnpackBuffer
    {
    public:
        DECLARE_MESSAGE_ID(MessageIDS::SetClientId);

        uint32_t ClientId = 0;

        int GetProcessingChannel() override { return RouteID::ControllHandler; }

        SendClientId(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
            Read(ClientId);
        }
    };

    class WorldInfo : public MessageUnpackBuffer
    {
    public:
        DECLARE_MESSAGE_ID(MessageIDS::WorldInfo);

        float BoundsX = 0;
        float BoundsY = 0;

        int GetProcessingChannel() override { return RouteID::GameHandler; }

        WorldInfo(ENetPacket* packet) : MessageUnpackBuffer(packet)
        {
            Read(BoundsX);
            Read(BoundsY);
        }
    };
}