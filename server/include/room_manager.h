#pragma once

#include "connected_client.h"

#include <string>
#include <unordered_map>

class Room
{
public:
	std::string Name;

	std::unordered_map<uint32_t, ConnectedClient*> Players;
};

namespace RoomManager
{
	void Init();
	Room* CreateRoom(const std::string& name);
	Room* GetRoom(const std::string& name);
	void DestroyRoom(const std::string& name);
}
