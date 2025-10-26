#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

namespace Hashes
{
    class GUID
    {
        uint16_t Data[8] = { 0 };

    public:
        static GUID New();

        static constexpr GUID Invalid()
        {
            return GUID();
        }

        void Parse(std::string_view text);

        bool operator==(const GUID& other) const;
        bool operator!=(const GUID& other) const;

        std::string ToString() const;
        static GUID FromString(std::string_view text);

        inline size_t Hash() const
        {
            std::size_t hash = 8;
            for (int i = 0; i < 8; i++)
                hash ^= Data[i] + 0x9e3779b9 + (hash << 6) + (hash >> 2);

            return hash;
        }

        size_t operator()(const GUID& guid) const
        {
            return guid.Hash();
        }
    };


}

template <>
struct std::hash<Hashes::GUID>
{
    std::size_t operator()(const Hashes::GUID& guid) const
    {
        return guid.Hash();
    }
};