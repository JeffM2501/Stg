#pragma once

#include "connected_client.h"

#include <string>
#include <unordered_map>

class Room
{
public:
	uint32_t ID;
	std::string Name;

	uint32_t ChatGroupId = 0;

	std::unordered_map<uint32_t, ConnectedClient*> Players;

	Room(uint32_t id, std::string_view name);
	~Room();
};

namespace RoomManager
{
	void Init();
	Room* CreateRoom(const std::string& name);
	Room* GetRoom(uint32_t id);
	Room* GetRoom(const std::string& name);
	void DestroyRoom(uint32_t id);
}
