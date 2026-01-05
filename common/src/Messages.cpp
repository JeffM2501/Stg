#include "messages.h"
#include <unordered_map>

namespace MessageUnpackFactories
{
    std::unordered_map<uint64_t, MessageUnapckBufferFactory> Factories;

    void RegisterFactory(uint64_t messageTypeID, MessageUnapckBufferFactory factory)
    {
        Factories.insert_or_assign(messageTypeID, factory);
    }

    MessageUnpackBufferPtr Unpack(ENetPacket* packet)
    {
        uint64_t id = MessageUnpackBuffer::GetMessageTypeID(packet);

        auto itr = Factories.find(id);
        if (itr == Factories.end())
            return nullptr;

        return itr->second(packet);
    }
}