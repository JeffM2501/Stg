#include "room_processor.h"
#include "room_manager.h"

#include "chat_system.h"

#include "messages/room_messages.h"

using namespace RoomManager;

Room::Room(uint32_t id, std::string_view name) 
	: ID(id)
	, Name(name)
{
	ChatGroupId = ChatSystem::CreateChatGroup(name);
}

Room::~Room()
{
	ChatSystem::DestroyChatGroup(ChatGroupId);
}

namespace RoomProcessor
{
	void AddPlayer(Room* room, ConnectedClient* player)
	{
		// send set room message to player and start the load process
        player->CurrentRoomID.store(room->ID);

		ChatSystem::AddUserToGroup(player, room->ChatGroupId);
		player->Send<RoomMessages::AddUserToRoom>(player->Peer->connectID);


        room->Players.insert_or_assign(player->Peer->connectID, player);
	}

	void RemovePlayer(Room* room, ConnectedClient* player)
	{
		player->CurrentRoomID.store(0);

		// send leave room message to other players
	}

	void ProcessMessage(Room* room, ConnectedClient* sender, std::unique_ptr<MessageBuffer> buffer)
	{

	}
}

