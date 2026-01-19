#include "messages.h"
#include <unordered_map>


namespace MessageFactories
{
    std::unordered_map<uint64_t, MessageBufferFactory> Factories;

    void RegisterFactory(uint64_t messageTypeID, MessageBufferFactory factory)
    {
        Factories.insert_or_assign(messageTypeID, factory);
    }

    MessageBufferPtr Unpack(ENetPacket* packet)
    {
        uint64_t id = MessageBuffer::GetMessageTypeID(packet);

        auto itr = Factories.find(id);
        if (itr == Factories.end())
            return nullptr;

        MessageBufferPtr msg = itr->second(packet);
        if (msg)
            msg->MessageTypeId = id;

        return msg;
    }
}