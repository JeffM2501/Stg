#pragma once

#include "connected_client.h"
#include "messages.h"
#include <memory>

class Room;

namespace RoomProcessor
{
	void AddPlayer(Room* room, ConnectedClient* player);
	void RemovePlayer(Room* room, ConnectedClient* player);

	void ProcessMessage(Room* room, ConnectedClient* sender, std::unique_ptr<MessageBuffer> buffer);
}