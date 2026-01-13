#define ENET_IMPLEMENTATION
#include "enet.h"
#undef ENET_IMPLEMENTATION

#include <cstdio>

#include "timed_callbacks.h"
#include "lifetime_token.h"

#include "connected_client.h"
#include "message_channels.h"

#include "messages.h"
#include "controll_messages.h"
#include "messages/chat_group_messages.h"

#include "client_database.h"
#include "message_router.h"
#include "chat_system.h"

static int constexpr MaxClients = 64;

TimedCallbackHost GlobalTimmer;

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

Tokens::TokenSource AppTokenSource;

int main()
{
	ChatSystem::Init();

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

	GlobalTimmer.Add("Heartbeat", 1, [](size_t)
		{
			ClientDB::DoForEachClient([](auto* client)
				{
					ChatSystem::SendServerMessage("Badump");
				});
		}, true);

	printf("Started a server...\n");

	ClientDB::OnNewConnection.Add([](void*, auto& client)
		{
			client->Send<Pack::SendClientId>(client->Peer->connectID);

			client->Send<Pack::WorldInfo>(300, 300);
		}, AppTokenSource.GetToken());

	ENetEvent event;

	auto lastTime = std::chrono::steady_clock::now();

	while (true)
	{
		auto now = std::chrono::steady_clock::now();
		auto delta = now - lastTime;
		lastTime = now;

		enet_host_service(server, &event, 1000);
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			ClientDB::NewConnection(event.peer);
			break;

		case ENET_EVENT_TYPE_RECEIVE:
			MessageRouter::PacketReceive(event.peer, event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			ClientDB::DestroyConnection(event.peer);
			break;

		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			ClientDB::DestroyConnection(event.peer);
			break;

		case ENET_EVENT_TYPE_NONE:
			break;
		}

		GlobalTimmer.Update(std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());
		ChatSystem::Process();

		ClientDB::DoForEachClient([](auto* client)
			{
				while (!client->OutboundPackets.Empty())
				{
					auto message = client->OutboundPackets.Pop();
					SendClientMessage(client, *message, int(message->Channel), message->Reliable, message->Ordered);
				}
			});
	}

	enet_host_destroy(server);
	enet_deinitialize();
	return 0;
}