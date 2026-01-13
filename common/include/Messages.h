#pragma once

#include "enet.h"

#include "message_channels.h"
#include "message_route_ids.h"

#include <memory>
#include <functional>
#include <string_view>
#include <span>

class MessageUnpackBuffer
{
protected:
    
    ENetPacket* Packet = nullptr;

    uint64_t ReadOffset = 0;

    std::span<uint8_t> ReadBuffer(size_t offset) const
    {
        offset += GetStartOffset();

        if (Packet == nullptr || Packet->data == nullptr || offset >= Packet->dataLength)
            return std::span<uint8_t>();

        uint32_t size = sizeof(uint32_t);

        if (offset + size > Packet->dataLength)
            return std::span<uint8_t>();

        size = *(reinterpret_cast<const uint32_t*>(Packet->data + offset));
        offset += sizeof(uint32_t);

        return std::span<uint8_t>(Packet->data + offset, size);
    }

    size_t ReadBufferSize(size_t offset) const
    {
        offset += GetStartOffset();

        if (Packet == nullptr || Packet->data == nullptr || offset >= Packet->dataLength)
            return 0;

        uint32_t size = sizeof(uint32_t);

        if (offset + size > Packet->dataLength)
            return 0;

        size = *(reinterpret_cast<const uint32_t*>(Packet->data + offset));
        return size + sizeof(uint32_t);
    }

    std::string_view ReadString(size_t offset)const
    {
        auto range = ReadBuffer(offset);
        return std::string_view((char*)(range.data()), range.size());
    }

    template<class T>
    const T* ReadValue(size_t offset) const
    {
        offset += GetStartOffset();

        if (Packet == nullptr || Packet->data == nullptr || offset >= Packet->dataLength)
            return nullptr;

        if (offset + sizeof(T) > Packet->dataLength)
            return nullptr;

        return reinterpret_cast<const T*>(Packet->data + offset);
    }

    template<class T>
    bool Read(T& value)
    {
        const T* pValue = ReadValue<T>(ReadOffset);
        if (pValue != nullptr)
        {
            value = *pValue;
            ReadOffset += sizeof(T);
        }

        return pValue != nullptr;
    }

    template<>
    bool Read(std::span<uint8_t>& value)
    {
        value = ReadBuffer(ReadOffset);
        ReadOffset += value.size() + sizeof(uint32_t);
        return value.size() > 0;
    }

    template<>
    bool Read(std::string_view& value)
    {
        std::span<uint8_t> buffer = ReadBuffer(ReadOffset);
        ReadOffset += value.size() + sizeof(uint32_t);

        value = std::string_view(reinterpret_cast<const char*>(buffer.data()), buffer.size());

        return buffer.size() > 0;
    }

    virtual size_t GetStartOffset() const { return 8; }

public:
    virtual int GetProcessingChannel() { return RouteID::SystemHandler; }
    uint64_t MessageTypeId = InvalidMessageTypeID;

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
    static uint64_t GetMessageTypeID(ENetPacket* packet)
    {
        if (packet->dataLength < 8)
            return InvalidMessageTypeID;

        return *reinterpret_cast<const uint64_t*>(packet->data);
    }
};

#define DECLARE_MESSAGE_ID(T) static uint64_t TypeId(){return T;}

namespace MessageUnpackFactories
{
    using MessageUnpackBufferPtr = std::unique_ptr<MessageUnpackBuffer>;
    using MessageUnapckBufferFactory = std::function<MessageUnpackBufferPtr(ENetPacket*)>;

    void RegisterFactory(uint64_t messageTypeID, MessageUnapckBufferFactory factory);
    MessageUnpackBufferPtr Unpack(ENetPacket* packet);
}

class MessagePackBuffer
{
protected:
    virtual size_t GetStartOffset() const { return 8; }

public:
    ENetPacket* Packet = nullptr;
    NetworkChannelIDs Channel = NetworkChannelIDs::Control;

    bool Reliable = true;
    bool Ordered = true;

    size_t WriteOffset = 0;

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
    void WriteValue(const T& value, size_t offset)
    {
        offset += GetStartOffset();

        if (Packet == nullptr || Packet->data == nullptr || offset >= Packet->dataLength)
            return;

        if (offset + sizeof(T) > Packet->dataLength)
            return;

        *(reinterpret_cast<T*>(Packet->data + offset)) = value;
    }

    template<class T>
    bool Write(const T& value)
    {
        size_t offset = WriteOffset + GetStartOffset();

        if (Packet == nullptr || Packet->data == nullptr || offset >= Packet->dataLength)
            return false;

        if (offset + sizeof(T) > Packet->dataLength)
            return false;

        *(reinterpret_cast<T*>(Packet->data + offset)) = value;
        WriteOffset += sizeof(T);
        return true;
    }

    static size_t GetBufferWriteSize(size_t bufferSize)
    {
        return bufferSize + sizeof(uint32_t);
    }

    bool WriteBufferValue(const void* buffer, size_t bufferSize, size_t offset)
    {
        offset += GetStartOffset();

        if (Packet == nullptr || Packet->data == nullptr || offset >= Packet->dataLength)
            return false;

        size_t writeSize = bufferSize + sizeof(uint32_t);

        if (offset + writeSize > Packet->dataLength)
            return false;

        uint32_t size = static_cast<uint32_t>(bufferSize);
        memcpy(Packet->data + offset, &size, sizeof(uint32_t));
        memcpy(Packet->data + offset + sizeof(uint32_t), buffer, bufferSize);

        return true;
    }

    bool WriteBuffer(const uint8_t* buffer, size_t bufferSize)
    {
        bool valid = WriteBufferValue(buffer, bufferSize, WriteOffset);
        if (valid)
        {
            WriteOffset += bufferSize + sizeof(uint32_t);
        }
        return valid;
    }

    template<>
    bool Write(const std::string_view& value)
    {
        return WriteBuffer(reinterpret_cast<const uint8_t*>(value.data()), value.size());
    }

    void WriteTypeID(size_t messageTypeID)
    {
        if (Packet == nullptr || Packet->data == nullptr || Packet->dataLength < GetStartOffset())
            return;

        *(reinterpret_cast<size_t*>(Packet->data)) = messageTypeID;
    }
};
