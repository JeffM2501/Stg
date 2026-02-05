#include "player_database.h"


namespace PlayerDatabase
{
    std::unordered_map<uint32_t, User> Users;

    void Init()
    {
        Users.clear();
    }

    void Cleanup()
    {
        Users.clear();
    }

    User* GetUserFromId(uint32_t id)
    {
        auto itr = Users.find(id);
        if (itr != Users.end())
            return &(itr->second);

        return nullptr;
    }
}