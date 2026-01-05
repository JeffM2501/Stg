#include "relative_buffer.h"

namespace ResourceBuffers
{
    void RelativeBufferBuilder::StartBuffer(size_t typeId, size_t bufferID)
    {
        if (CurrentBuffer)
            EndBuffer();

        if (!PendingBuffers.contains(typeId))
            PendingBuffers.emplace(typeId, std::unordered_map<size_t, BufferInfo>());

        auto& typeBuffers = PendingBuffers.at(typeId);

        typeBuffers.insert_or_assign(bufferID, BufferInfo{});
        CurrentBuffer = &typeBuffers.at(bufferID);
    }

    void RelativeBufferBuilder::AddField(size_t fieldId, void* bufferData, size_t size)
    {
        if (!CurrentBuffer)
            return;

        CurrentBuffer->ManifestOffsets.insert_or_assign(fieldId, CurrentBuffer->DataStorage.size());
        CurrentBuffer->DataStorage.insert(CurrentBuffer->DataStorage.end(), (uint8_t*)bufferData, (uint8_t*)bufferData + size);
    }

    void RelativeBufferBuilder::EndBuffer()
    {
        CurrentBuffer = nullptr;
    }

    //----------------------------------------------------------
    /*
    header
    Buffer Count
    Buffer Entries...
        Buffer Type ID
        Buffer ID
        Buffer Start Offset

    Buffer Body
        Field Count
        Field Entries...
            Field ID
            Field Offset
        Field Data...
    */

    void RelativeBufferBuilder::Finalize(BufferWriter& Writer)
    {
        Writer.Write(PendingBuffers.size());

        size_t headerSize = 8 + PendingBuffers.size() * 24;

        size_t offset = headerSize + 8;

        for (auto& [typeId, bufferList] : PendingBuffers)
        {
            for (auto& [bufferID, bufferInfo] : bufferList)
            {
                Writer.Write(typeId);
                Writer.Write(bufferID);
                Writer.Write(offset);

                size_t bufferSize = 8 + bufferInfo.ManifestOffsets.size() * 16 + bufferInfo.DataStorage.size();
                offset += bufferSize;
            }
        }

        for (auto& [typeId, bufferList] : PendingBuffers)
        {
            for (auto& [bufferID, bufferInfo] : bufferList)
            {
                Writer.Write(bufferInfo.ManifestOffsets.size());
                for (auto& [fieldID, fieldOffset] : bufferInfo.ManifestOffsets)
                {
                    Writer.Write(fieldID);
                    Writer.Write(fieldOffset);
                }

                Writer.Write(bufferInfo.DataStorage.data(), bufferInfo.DataStorage.size());
            }
        }
    }

    RelativeBufferContainer RelativeBufferReader::Read(std::span<uint8_t> buffer)
    {
        RelativeBufferContainer container;
        container.BufferData = buffer;
        MemoryBufferReader reader(container.BufferData);

        auto count = reader.Read<size_t>();

        for (size_t i = 0; i < count; i++)
        {
            size_t typeId = reader.Read<size_t>();
            size_t bufferID = reader.Read<size_t>();
            size_t bufferOffset = reader.Read<size_t>();
            size_t currentOffset = reader.Offset;
            reader.Offset = bufferOffset;
            RelativeBuffer relBuffer;
            size_t fieldCount = reader.Read<size_t>();
            for (size_t f = 0; f < fieldCount; f++)
            {
                size_t fieldID = reader.Read<size_t>();
                size_t fieldOffset = reader.Read<size_t>();
                relBuffer.ManifestOffsets.insert_or_assign(fieldID, fieldOffset);
            }
            size_t dataStart = reader.Offset;
            size_t dataEnd = (i + 1 < count) ? reader.Read<size_t>() : buffer.size();
            relBuffer.Buffer = container.BufferData.subspan(dataStart, dataEnd - dataStart);
            container.BufferManifest[typeId].insert_or_assign(bufferID, relBuffer);
            reader.Offset = currentOffset;
        }

        return container;
    }
}