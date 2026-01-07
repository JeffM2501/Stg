#pragma once

#include "connected_client.h"
#include "Events.h"
#include "enet.h"

#include <functional>

namespace ClientDB
{
	extern Events::EventSource<ConnectedClient*> OnNewConnection;
	extern Events::EventSource<ConnectedClient*> OnDestroyConnection;

    extern Events::EventSource<ConnectedClient*> OnClientStateChanged;

	void NewConnection(ENetPeer* peer);
	void DestroyConnection(ENetPeer* peer);

	ConnectedClient* GetClient(ENetPeer* peer);
	ConnectedClient* GetClient(uint64_t clientID);

	void DoForEachClient(std::function<void(ConnectedClient*)> func);
}