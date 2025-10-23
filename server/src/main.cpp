#define ENET_IMPLEMENTATION
#include "enet.h"

#include <cstdio>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <deque>
#include <memory>

template <class T>
class AtomicQueue : public std::deque<T>
{
private:
	std::mutex Mutex;

public:
	std::lock_guard<std::mutex> GetLock()
	{
		return std::lock_guard<std::mutex>(Mutex);
    }

	bool Empty()
	{
		auto lock = GetLock();

		return Size() == 0;
	}

	T Pop()
	{
		auto lock = GetLock();
		T val = front();
		pop_front();
        return val;
	}

	void Push(const T& val)
	{
		auto lock = GetLock();
		push_back(val);
    }
};

enum class NetworkChannelIDs : uint8_t
{
	Control = 0,
	Chat = 1,
	Assets = 2,
	Updates = 3,
	Count = 4
};

static int constexpr MaxClients = 64;

enum class ClientState : uint8_t
{
	Unknown = 0,
	Negotiating = 1,
	AssetLimbo = 2,
	GameLimbo = 3,
	Playing = 4,
	Tooling = 5,
	Admin = 6,
};

struct ConnectedClient
{
	ClientState State = ClientState::Unknown;

	AtomicQueue<ENetPacket*> InboundPackets;
	AtomicQueue<ENetPacket*> OutboundPackets;
};

std::unordered_map<uint64_t, std::shared_ptr<ConnectedClient>> Clients;

void HandleNewConnection(ENetPeer* peer)
{
	Clients.insert_or_assign(peer->connectID, std::make_shared<ConnectedClient>());
}

void HandleDestroyConnection(ENetPeer* peer)
{
	auto itr = Clients.find(peer->connectID);
	if (itr == Clients.end())
		return;

	Clients.erase(itr);
}

void HandlePacketReceive(ENetPeer* peer, ENetPacket* packet, int channelID)
{
    printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
		packet->dataLength,
		packet->data,
		peer->data,
		channelID);

	auto itr = Clients.find(peer->connectID);
	if (itr == Clients.end())
	{
		enet_packet_destroy(packet);
        return;
	}
	itr->second->InboundPackets.Push(packet);
}

int main()
{
	if (enet_initialize() != 0) 
	{
		printf("An error occurred while initializing ENet.\n");
		return 1;
	}
	ENetAddress address = { 0 };

	address.host = ENET_HOST_ANY; /* Bind the server to the default localhost.     */
	address.port = 7777; /* Bind the server to port 7777. */

	/* create a server */
	ENetHost* server = enet_host_create(&address, MaxClients, int(NetworkChannelIDs::Count), 0, 0);

	if (server == nullptr)
	{
		printf("An error occurred while trying to create an ENet server host.\n");
		return 1;
	}

	printf("Started a server...\n");

	ENetEvent event;

	/* Wait up to 1000 milliseconds for an event. (WARNING: blocking) */
	while (true)
	{
		enet_host_service(server, &event, 1000);
		switch (event.type) 
		{
		case ENET_EVENT_TYPE_CONNECT:
			printf("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
			/* Store any relevant client information here. */
			event.peer->data = "Client information";
			HandleNewConnection(event.peer);
			break;

		case ENET_EVENT_TYPE_RECEIVE:
			HandlePacketReceive(event.peer, event.packet, event.channelID);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			printf("%s disconnected.\n", event.peer->data);
			/* Reset the peer's client information. */
			event.peer->data = NULL;
			HandleDestroyConnection(event.peer);
			break;

		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			printf("%s disconnected due to timeout.\n", event.peer->data);
			/* Reset the peer's client information. */
			event.peer->data = NULL;
			HandleDestroyConnection(event.peer);
			break;

		case ENET_EVENT_TYPE_NONE:
			break;
		}
	}

	enet_host_destroy(server);
	enet_deinitialize();
	return 0;
}