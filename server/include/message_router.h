#pragma once

#include "client_database.h"
#include "messages.h"
#include "enet.h"

#include <functional>

namespace MessageRouter
{
	void PacketReceive(ENetPeer* peer, ENetPacket* packet);
	void RegisterRouteHandler(int handlerId, std::function<void(ConnectedClient* client, std::unique_ptr<MessageBuffer> buffer)> func);
}