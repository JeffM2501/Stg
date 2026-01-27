#include "external/fix_win32_compatibility.h"

#define ENET_IMPLEMENTATION
#include "enet.h"
#undef ENET_IMPLEMENTATION

#include "connection.h"

#include "messages.h"
#include "message_channels.h"
#include "game.h"
#include "chat_client.h"

/*#include "world_info.h"*/

#include "messages/control_messages.h"
#include "messages/chat_group_messages.h"
#include "messages/chat_messages.h"

#include <unordered_map>

namespace Connection
{
	ENetHost* Client = {};
	ENetPeer* ServerPeer = {};
	bool Connected = false;

	uint32_t ClientId = 0;

	Events::EventSource<uint32_t> OnConnected;
	Events::EventSource<uint32_t> OnConnectionComplete;

	std::unordered_map<size_t, std::unique_ptr<MessageHandler>> MessageHandlers;

	void RegisterMessages()
	{
		ChatGroupMessages::Register();
		ChatMessages::Register();
		ControlMessages::Register();
	}

	void RegisterDefaultHandlers()
	{
		RegisterHandler<ControlMessages::SendClientId>().ProcessFunc = [](const ControlMessages::SendClientId& message)
			{
				ClientId = message.ClientID();
                ChatClient::SetUserID(ClientId);
				OnConnectionComplete.Invoke(nullptr, ClientId);

			};
	}

	void Init()
	{
		RegisterMessages();

		enet_initialize();
		RegisterDefaultHandlers();

		Client = enet_host_create(nullptr, 1, int(NetworkChannelIDs::Count), 0, 0);
	}

	void Cleanup()
	{
		// unload resources
		if (Client)
		{
			enet_host_destroy(Client);
			Client = nullptr;
		}
	}

	bool IsConnected()
	{
		return Connected;
	}

	bool Connect(std::string_view host)
	{
		ENetAddress address = { 0 };
		ENetEvent event = {};
		ENetPeer* peer = nullptr;

		enet_address_set_host(&address, host.data());
		address.port = 7777;
		ServerPeer = enet_host_connect(Client, &address, int(NetworkChannelIDs::Count), 0);

		return ServerPeer != nullptr;
	}

	uint32_t GetClientId()
	{
		return ClientId;
	}

	void Send(MessageBuffer& message)
    {
        if (message.Packet == nullptr)
            return;
        enet_uint32 flags = 0;
        if (message.Reliable)
            flags |= ENET_PACKET_FLAG_RELIABLE;
        if (!message.Ordered)
            flags |= ENET_PACKET_FLAG_UNSEQUENCED;

        message.Packet->flags = flags;

        enet_peer_send(ServerPeer, int(message.Channel), message.Packet);
    }

    void ServiceNetwork()
	{
		if (Client == nullptr)
			return;

		ENetEvent event = {};
		if (enet_host_service(Client, &event, 10))
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				Connected = true;
				// connected to server
				OnConnected.Invoke(nullptr, event.peer->connectID);
				break;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				size_t messageTypeID = MessageBuffer::GetMessageTypeID(event.packet);
				auto handlerItr = MessageHandlers.find(messageTypeID);
				if (handlerItr != MessageHandlers.end())
				{
					handlerItr->second->Process(event.packet);
				}
				else
				{
					enet_packet_destroy(event.packet);
				}
			}
			break;

			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			case ENET_EVENT_TYPE_DISCONNECT:
				// disconnected from server
				GameStageManager::Quit();
				Connected = false;
				enet_host_destroy(Client);
				Client = nullptr;
				break;
			default:
				break;
			}
		}
	}
}