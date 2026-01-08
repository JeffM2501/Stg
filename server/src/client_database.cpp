#include "client_database.h"

#include <memory>
#include <mutex>

void ConnectedClient::SetState(ClientState state)
{
	State.store(state);
	ClientDB::OnClientStateChanged.Invoke(this, this);
}

ClientState ConnectedClient::GetState()
{
	return State.load();
}

namespace ClientDB
{
	Events::EventSource<ConnectedClient*> OnNewConnection;
	Events::EventSource<ConnectedClient*> OnDestroyConnection;
	Events::EventSource<ConnectedClient*> OnClientStateChanged;

	std::unordered_map<uint64_t, std::shared_ptr<ConnectedClient>> Clients;

	std::mutex ClientsMutex;

	void NewConnection(ENetPeer* peer)
	{
		auto client = std::make_shared<ConnectedClient>(peer);
		{
			std::lock_guard<std::mutex> lock(ClientsMutex);
		
			peer->data = client.get();
			Clients.insert_or_assign(peer->connectID, client);
		}
		OnNewConnection.Invoke(nullptr, client.get());
	}

	void DestroyConnection(ENetPeer* peer)
	{
		std::shared_ptr<ConnectedClient> client;
		{
			std::lock_guard<std::mutex> lock(ClientsMutex);

			std::unordered_map<uint64_t, std::shared_ptr<ConnectedClient>>::iterator itr = Clients.find(peer->connectID);
			if (itr == Clients.end())
				return;

			client = itr->second;
			Clients.erase(itr);
		}

		if (client)
			OnDestroyConnection.Invoke(nullptr, client.get());
	}

	ConnectedClient* GetClient(ENetPeer* peer)
	{
		return GetClient(peer->connectID);
	}

	ConnectedClient* GetClient(uint64_t clientID)
	{
		std::lock_guard<std::mutex> lock(ClientsMutex);
		std::unordered_map<uint64_t, std::shared_ptr<ConnectedClient>>::iterator itr = Clients.find(clientID);
		if (itr == Clients.end())
			return nullptr;

		return itr->second.get();
	}

	void DoForEachClient(std::function<void(ConnectedClient*)> func)
	{
		if (!func)
			return;

		std::lock_guard<std::mutex> lock(ClientsMutex);
		for (auto& [id, client] : Clients)
		{
			func(client.get());
		}
	}
}