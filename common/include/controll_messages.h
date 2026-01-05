#pragma once

#include "messages.h"
#include "message_ids.h"

namespace Pack
{
    class SendClientId : public MessagePackBuffer
    {
    public:
        SendClientId(uint32_t clientId)
        {
            Channel = NetworkChannelIDs::Control;
            AllocatePacket(sizeof(uint32_t));
            WriteTypeID(MessageIDS::SetClientId);
            SetClientId(clientId);
        }

        void SetClientId(uint32_t clientId)
        {
            WriteValue<uint32_t>(clientId, 0);
        }
    };

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