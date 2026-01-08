#pragma once

#include "connected_client.h"

#include <string>
#include <unordered_map>

class Room
{
public:
	size_t ID;
	std::string Name;

	size_t ChatGroupId = 0;

	std::unordered_map<uint32_t, ConnectedClient*> Players;

	Room(size_t id, std::string_view name);
	~Room();
};

namespace RoomManager
{
	void Init();
	Room* CreateRoom(const std::string& name);
	Room* GetRoom(size_t id);
	Room* GetRoom(const std::string& name);
	void DestroyRoom(size_t id);
}
