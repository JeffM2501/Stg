#define ENET_IMPLEMENTATION
#include "enet.h"
#undef ENET_IMPLEMENTATION

#include <cstdio>
#include <unordered_map>
#include <memory>

#include "AtomicQueue.h"
#include "ConnectedClient.h"
#include "MessageChannels.h"

#include "Messages.h"
#include "MessageIDs.h"

class SendClientId : public MessagePackBuffer
{
public:
	SendClientId(uint32_t clientId)
	{
		AllocatePacket(sizeof(uint32_t));
		WriteTypeID(MessageIDS::SetClientId);
    }

	void SetClientId(uint32_t clientId)
	{
		WriteValue<uint32_t>(clientId, 0);
	}
};

static int constexpr MaxClients = 64;

std::unordered_map<uint64_t, std::shared_ptr<ConnectedClient>> Clients;

using MessageChannelProcessor = std::function<void(ConnectedClient*, ENetPacket*, int)>;
std::unordered_map<NetworkChannelIDs, MessageChannelProcessor> ChannelProcessors;

void HandleNewConnection(ENetPeer* peer)
{
	Clients.insert_or_assign(peer->connectID, std::make_shared<ConnectedClient>(peer));
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
	auto itr = Clients.find(peer->connectID);
	if (itr == Clients.end())
	{
		enet_packet_destroy(packet);
        return;
	}
	itr->second->InboundPackets.Push(packet);
}

void SendClientMessage(ConnectedClient* client, MessagePackBuffer& message, int channel, bool reliable, bool ordered)
{ 
	if (message.Packet == nullptr)
		return;
	enet_uint32 flags = 0;
	if (reliable)
		flags |= ENET_PACKET_FLAG_RELIABLE;
	if (!ordered)
		flags |= ENET_PACKET_FLAG_UNSEQUENCED;

    message.Packet->flags = flags;

    enet_peer_send(client->Peer, channel, message.Packet);
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

	while (true)
	{
		enet_host_service(server, &event, 1000);
		switch (event.type) 
		{
		case ENET_EVENT_TYPE_CONNECT:
			event.peer->data = "Client information";
			HandleNewConnection(event.peer);
		break;

		case ENET_EVENT_TYPE_RECEIVE:
		{
            auto itr = ChannelProcessors.find(NetworkChannelIDs(event.channelID));
			if (itr != ChannelProcessors.end())
			{
				itr->second(Clients[event.peer->connectID].get(), event.packet, event.channelID);
			}
			else
			{
				HandlePacketReceive(event.peer, event.packet, event.channelID);
			}
        }
        break;

		case ENET_EVENT_TYPE_DISCONNECT:
			event.peer->data = NULL;
			HandleDestroyConnection(event.peer);
		break;

		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
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