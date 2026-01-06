#include "chat_system.h"

#include "client_database.h"
#include "message_route_ids.h"
#include "message_router.h"

#include "chat_messages.h"

#include "lifetime_token.h"

#include <unordered_map>
#include <set>
#include <string>
#include <random>
#include <chrono>


namespace ChatSystem
{
	static Tokens::TokenSource ChatLifetimeToken;

	std::vector<std::string> NameParts;

	std::mt19937 RandEngine(uint32_t(std::chrono::system_clock::now().time_since_epoch().count()));

	std::string_view GetRandomName()
	{
		std::uniform_int_distribution<size_t> dist(0, NameParts.size()-1);
		return NameParts[dist(RandEngine)];
	}

	void InitNames()
	{
		NameParts = {
			"Red", "Blue", "Green", "Yellow", "Fast", "Slow", "Happy", "Sad",
			"Cat", "Dog", "Bird", "Fish", "Lion", "Tiger", "Bear", "Wolf",
			"Sky", "Ocean", "Mountain", "River", "Forest", "Desert", "Cloud", "Star",
			"Underpants", "Gnome", "Thunder", "Flare"
		};
	}

	struct ClientChatInfo
	{
		ConnectedClient* Client;
		std::string Name;
	};

	std::unordered_map<uint32_t, ClientChatInfo> ClientChatInfos;

	uint32_t FindClientIdByName(const std::string& name)
	{
		for (const auto& [id, info] : ClientChatInfos)
		{
			if (info.Name == name)
				return id;
		}
		return uint32_t(-1);
	}

	std::string GenerateGuestName()
	{
		static uint32_t GuestCounter = 0;

		GuestCounter++;
		std::string name;
		do
		{
			name = GetRandomName();
			name += GetRandomName();
			name += std::to_string(GuestCounter++);

		} while (FindClientIdByName(name) != uint32_t(-1));
		return name;
	}

	void ProcessChatMessage(ConnectedClient& sender, Unpack::ServerTextMessage* message)
	{
		// route to everyone else in the channel
	}

	void RouteMessage(ConnectedClient* client, std::unique_ptr<MessageUnpackBuffer> buffer)
	{
		switch (buffer->MessageTypeId)
		{
		case MessageIDS::ServerTextMessage:
			ProcessChatMessage(*client, static_cast<Unpack::ServerTextMessage*>(buffer.get()));
			break;
		}
	}

	void NewConnection(void* sender, ConnectedClient* client)
	{
		ClientChatInfo newClient;
		newClient.Client = client;
		newClient.Name = GenerateGuestName();
		ClientChatInfos.insert_or_assign(client->Peer->connectID, newClient);

		// tell everyone the name
		ClientDB::DoForEachClient([&newClient](ConnectedClient* other)
			{
				other->Send<Pack::ServerAddChatUser>(newClient.Client->Peer->connectID, newClient.Name);
			});

		std::string welcomeMessage = "Welcome " + newClient.Name;
		client->Send<Pack::ServerTextMessage>(welcomeMessage.c_str());
	}

	void DestroyConnection(void* sender, ConnectedClient* client)
	{
		auto itr = ClientChatInfos.find(client->Peer->connectID);
		if (itr == ClientChatInfos.end())
			return;

		auto id = itr->first;
		ClientChatInfos.erase(itr);

		// tell everyone who left
		ClientDB::DoForEachClient([id](ConnectedClient* other)
			{
				other->Send<Pack::ServerRemoveChatUser>(id);
			});
	}

	void Init()
	{
		InitNames();

		MessageRouter::RegisterRouteHandler(RouteID::ChatHandler, RouteMessage);

		ClientDB::OnNewConnection.Add(NewConnection, ChatLifetimeToken.GetToken());
		ClientDB::OnDestroyConnection.Add(DestroyConnection, ChatLifetimeToken.GetToken());
	}

	void Process()
	{
		// any automated or outbound processing
	}
}