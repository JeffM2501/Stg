#include "room_manager.h"
#include "message_router.h"
#include "message_route_ids.h"

#include <memory>

namespace RoomManager
{
	Room* DefaultRoom = nullptr;
	std::unordered_map<std::string, std::unique_ptr<Room>> Rooms;

	void RouteGameplayMessage(ConnectedClient* sender, std::unique_ptr<MessageUnpackBuffer> buffer)
	{
		// Implementation for routing gameplay messages
	}
	void Init()
	{
		MessageRouter::RegisterRouteHandler(RouteID::GameHandler, RouteGameplayMessage);
	}

	Room* CreateRoom(const std::string& name)
	{
		auto room = std::make_unique<Room>();
		room->Name = name;
		if (DefaultRoom == nullptr)
			DefaultRoom = room.get();
		Rooms.insert_or_assign(name, std::move(room));
		return Rooms[name].get();
	}

	Room* GetRoom(const std::string& name)
	{
		auto itr = Rooms.find(name);
		if (itr != Rooms.end())
			return itr->second.get();
		return nullptr;
	}

	void DestroyRoom(const std::string& name)
	{
		Room* room = GetRoom(name);
		if (!room)
			return;

		// kill everyone in the room and shunt them to the default

		for (auto& [id, client] : room->Players)
		{
			if (DefaultRoom && client)
			{
				// send a transfer message
				// set them to the inital state
				DefaultRoom->Players.insert_or_assign(id, client);
			}
		}

		Rooms.erase(name);
	}
}