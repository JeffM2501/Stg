#include "room_manager.h"
#include "message_router.h"
#include "message_route_ids.h"
#include "lifetime_token.h"

#include "room_processor.h"

#include <memory>

namespace RoomManager
{
	Room* DefaultRoom = nullptr;
	std::unordered_map<size_t, std::unique_ptr<Room>> Rooms;

	size_t LastRoomID = 0;

	static Tokens::TokenSource RoomManagerLifetimeToken;

	void RouteGameplayMessage(ConnectedClient* sender, std::unique_ptr<MessageUnpackBuffer> buffer)
	{
		if (!sender)
			return;

		auto* room = GetRoom(sender->CurrentRoomID);
		if (!room)
			return;

		RoomProcessor::ProcessMessage(room, sender, std::move(buffer));
	}

	void HandleStateChange(void*, ConnectedClient* client)
	{
		if (client->GetState() == ClientState::AssetLimbo)
		{
			if (client->CurrentRoomID.load() == 0 && DefaultRoom)
			{
				RoomProcessor::AddPlayer(DefaultRoom, client);
			}
		}
	}

	void Init()
	{
		CreateRoom("Lobby");

		MessageRouter::RegisterRouteHandler(RouteID::GameHandler, RouteGameplayMessage);

		ClientDB::OnClientStateChanged.Add(HandleStateChange, RoomManagerLifetimeToken.GetToken());
	}

	Room* CreateRoom(const std::string& name)
	{
		LastRoomID++;

		auto room = std::make_unique<Room>(LastRoomID, name);

		Room* ptr = room.get();

		if (DefaultRoom == nullptr)
			DefaultRoom = ptr;
		Rooms.insert_or_assign(LastRoomID, std::move(room));
		return ptr;
	}

	Room* GetRoom(size_t id)
	{
		auto itr = Rooms.find(id);
		if (itr == Rooms.end())
			return nullptr;

		return itr->second.get();
	}

	Room* GetRoom(const std::string& name)
	{
		for (auto& [id, room] : Rooms)
		{
			if (room->Name == name)
				return room.get();
		}
		return nullptr;
	}

	void DestroyRoom(size_t id)
	{
		Room* room = GetRoom(id);
		if (!room)
			return;

		// kill everyone in the room and shunt them to the default
		for (auto& [id, client] : room->Players)
		{
			RoomProcessor::RemovePlayer(room, client);

			if (DefaultRoom && client)
			{
				// shunt them to the default room
				RoomProcessor::AddPlayer(DefaultRoom, client);
			}
		}

		Rooms.erase(id);
	}
}


