#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "guid.h"

#include <random>

namespace Hashes
{
    static std::random_device RandomDevice;  // a seed source for the random number engine
    static std::mt19937 Generator(RandomDevice()); // mersenne_twister_engine seeded with rd()
    static std::uniform_int_distribution<> Distribution(std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max());


    GUID GUID::New()
    {
        GUID guid;
        for (int i = 0; i < 8; i++)
        {
            guid.Data[i] = Distribution(Generator);
        }

        return guid;
    }

    void GUID::Parse(std::string_view text)
    {
        if (sscanf(text.data(), "%04hX%04hX-%04hX-%04hX-%04hX-%04hX%04hX%04hX", &Data[0], &Data[1], &Data[2], &Data[3], &Data[4], &Data[5], &Data[6], &Data[7]) != 8)
            *this = GUID::Invalid();
    }

    std::string GUID::ToString() const
    {
        static char buffer[64] = { 0 };
        sprintf(buffer, "%04hX%04hX-%04hX-%04hX-%04hX-%04hX%04hX%04hX", Data[0], Data[1], Data[2], Data[3], Data[4], Data[5], Data[6], Data[7]);
        return std::string(buffer);
    }

    GUID GUID::FromString(std::string_view text)
    {
        GUID guid;
        guid.Parse(text);
        return guid;
    }

    bool GUID::operator!=(const GUID& other) const
    {
        for (int i = 0; i < 8; i++)
        {
            if (Data[i] != other.Data[i])
                return true;
        }
        return false;
    }

    bool GUID::operator==(const GUID& other) const
    {
        for (int i = 0; i < 8; i++)
        {
            if (Data[i] != other.Data[i])
                return false;
        }
        return true;
    }
}