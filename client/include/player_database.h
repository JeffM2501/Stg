#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

namespace PlayerDatabase
{
    struct User
    {
        std::string Name;
        uint32_t ID = 0;
        uint32_t RoomID = 0;
        bool Active = true;
    };

    extern std::unordered_map<uint32_t, User> Users;
    User* GetUserFromId(uint32_t id);

    void Init();
    void Cleanup();
}