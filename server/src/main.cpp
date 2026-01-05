#include "external/fix_win32_compatibility.h"

#define ENET_IMPLEMENTATION
#include "enet.h"
#undef ENET_IMPLEMENTATION

#include <cstdio>
#include <unordered_map>
#include <memory>

#include "atomic_queue.h"
#include "timed_callbacks.h"

#include "connected_client.h"
#include "message_channels.h"

#include "messages.h"
#include "controll_messages.h"
#include "chat_messages.h"

static int constexpr MaxClients = 64;

std::unordered_map<uint64_t, std::shared_ptr<ConnectedClient>> Clients;

using MessageChannelProcessor = std::function<void(ConnectedClient*, ENetPacket*, int)>;
std::unordered_map<NetworkChannelIDs, MessageChannelProcessor> ChannelProcessors;

TimedCallbackHost GlobalTimmer;

void HandleNewConnection(ENetPeer* peer)
{
    auto client = std::make_shared<ConnectedClient>(peer);
    peer->data = client.get();
    Clients.insert_or_assign(peer->connectID, client);

    client->Send<Pack::SendClientId>(client->Peer->connectID);
    client->Send<Pack::ServerTextMessage>("Welcome Human!");

    client->Send<Pack::WorldInfo>(300, 300);
}

void HandleDestroyConnection(ENetPeer* peer)
{
    auto itr = Clients.find(peer->connectID);
    if (itr == Clients.end())
        return;

    Clients.erase(itr);
}

void HandlePacketReceive(ENetPeer* peer, ENetPacket* packet, int /*channelID*/)
{
    auto itr = Clients.find(peer->connectID);
    if (itr == Clients.end())
    {
        enet_packet_destroy(packet);
        return;
    }
    itr->second->InboundPackets.Push(std::move(packet));
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

    GlobalTimmer.Add("Heartbeat", 1, [](size_t)
        {
            for (auto& [id, client] : Clients)
            {
                client->Send<Pack::ServerTextMessage>("Badump", 1);
            }
        }, true);

    printf("Started a server...\n");

    ENetEvent event;

    auto lastTime = std::chrono::steady_clock::now();

    while (true)
    {
        auto now = std::chrono::steady_clock::now();
        auto delta = now - lastTime;
        lastTime = now;

        GlobalTimmer.Update(std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());

        enet_host_service(server, &event, 1000);
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
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
            HandleDestroyConnection(event.peer);
            break;

        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            HandleDestroyConnection(event.peer);
            break;

        case ENET_EVENT_TYPE_NONE:
            break;
        }

        for (auto& [id, client] : Clients)
        {
            while (!client->OutboundPackets.Empty())
            {
                auto message = client->OutboundPackets.Pop();
                SendClientMessage(client.get(), *message, int(message->Channel), message->Reliable, message->Ordered);
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();
    return 0;
}