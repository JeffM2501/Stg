#pragma once

#include "enet.h"

class MessageUnpackBuffer
{
protected:
    ENetPacket* Packet = nullptr;

    template<class T>
    const T* ReadValue(size_t offset)
    {
        if (Packet == nullptr || Packet->data == nullptr || offset >= Packet->dataLength)
            return nullptr;

        if (offset + sizeof(T) > Packet->dataLength)
            return nullptr;

        return reinterpret_cast<const T*>(Packet->data + offset);
    }

public:
    MessageUnpackBuffer(ENetPacket* packet) : Packet(packet) {}
    virtual  ~MessageUnpackBuffer()
    {
        if (Packet)
        {
            enet_packet_destroy(Packet);
            Packet = nullptr;
        }
    }

    static constexpr size_t InvalidMessageTypeID = SIZE_MAX;
    static size_t GetMessageTypeID(ENetPacket* packet)
    {
        if (packet->dataLength < 8)
            return InvalidMessageTypeID;

        return *reinterpret_cast<const size_t*>(packet->data);
    }
};

class MessagePackBuffer
{
public:
    ENetPacket* Packet = nullptr;

    virtual void AllocatePacket(size_t size)
    {
        if (Packet)
        {
            enet_packet_destroy(Packet);
            Packet = nullptr;
        }

        Packet = enet_packet_create(nullptr, size + 8, 0);
    }

    template<class T>
    void WriteValue(const T& value,  size_t offset)
    {
        offset += 8;

        if (Packet == nullptr || Packet->data == nullptr || offset >= Packet->dataLength)
            return;

        if (offset + sizeof(T) > Packet->dataLength)
            return;

        *(reinterpret_cast<T*>(Packet->data + offset)) = value;
    }

    void WriteTypeID(size_t messageTypeID)
    {
        if (Packet == nullptr || Packet->data == nullptr || Packet->dataLength < 8)
            return;

        *(reinterpret_cast<size_t*>(Packet->data)) = messageTypeID;
    }
};