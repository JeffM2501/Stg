#pragma once

#include <span>
#include <string_view>
#include <unordered_map>

namespace ResourceBuffers
{
    struct RelativeBuffer
    {
        // list of fields by NameID and their offsets in the buffer
        std::unordered_map<size_t, size_t> ManifestOffsets;
        std::span<uint8_t> Buffer;
    };

    struct RelativeBufferContainer
    {
        // buffers sorted by type
        std::unordered_map<size_t, std::unordered_map<size_t, RelativeBuffer>> BufferManifest;

        std::span<uint8_t> BufferData;
    };

    class BufferWriter
    {
    public:
        virtual void Write(const void* data, size_t size) = 0;

        template<class T>
        void Write(const T& obj)
        {
            Write(&obj, sizeof(T));
        }
    };

    class MemoryBufferWriter : public BufferWriter
    {
    public:
        std::vector<uint8_t> Buffer;
        void Write(const void* data, size_t size) override
        {
            Buffer.insert(Buffer.end(), (const uint8_t*)data, (const uint8_t*)data + size);
        }
    };

    class MemoryBufferReader
    {
    public:
        std::span<uint8_t> Buffer;
        size_t Offset = 0;

        MemoryBufferReader(std::span<uint8_t>& buffer) : Buffer(buffer) {}

        void Read(const void* data, size_t size)
        {
            memcpy((void*)data, Buffer.data() + Offset, size);
            Offset += size;
        }

        template<class T>
        T Read()
        {
            T value;
            Read(&value, sizeof(T));
            return value;
        }
    };

    class RelativeBufferBuilder
    {
        struct BufferInfo
        {
            std::unordered_map<size_t, size_t> ManifestOffsets;
            std::vector<uint8_t> DataStorage;
        };

        std::unordered_map<size_t, std::unordered_map<size_t, BufferInfo>> PendingBuffers;

        BufferInfo* CurrentBuffer = nullptr;

    public:
        void StartBuffer(size_t typeId, size_t bufferID);

        void AddField(size_t fieldId, void* bufferData, size_t size);

        template<class T>
        void AddField(std::string_view name, const T& value)
        {
            static std::hash<std::string_view> hasher;
            AddField(hasher(name), &value, sizeof(T));
        }

        void EndBuffer();

        void Finalize(BufferWriter& Writer);
    };

    class RelativeBufferReader
    {
        RelativeBufferContainer Read(std::span<uint8_t> buffer);
    };
}