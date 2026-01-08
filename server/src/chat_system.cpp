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
		std::uniform_int_distribution<size_t> dist(0, NameParts.size() - 1);
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
		std::set<size_t> JoinedGroups;
	};

	std::unordered_map<uint32_t, ClientChatInfo> ClientChatInfos;

	struct ChatGroup
	{
	public:
		std::string Name;
		std::vector<ClientChatInfo*> Members;
	};

	std::mutex GroupsMutex;
	std::unordered_map<size_t, ChatGroup> ChatGroups;

	ClientChatInfo* GetChatInfo(ConnectedClient* client)
	{
		auto itr = ClientChatInfos.find(client->Peer->connectID);
		if (itr == ClientChatInfos.end())
			return nullptr;
		return &itr->second;
	}

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

		SendServerMessage("Welcome " + newClient.Name, client);

		client->SetState(ClientState::AssetLimbo);
	}

	void DestroyConnection(void* sender, ConnectedClient* client)
	{
		auto itr = ClientChatInfos.find(client->Peer->connectID);
		if (itr == ClientChatInfos.end())
			return;

		for (auto id : itr->second.JoinedGroups)
		{
			RemoveUserFromGroup(client, id);
		}

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

	size_t CreateChatGroup(std::string_view groupName)
	{
		std::lock_guard<std::mutex> lock(GroupsMutex);

		size_t id = std::hash<std::string_view>{}(groupName);

		if (ChatGroups.find(id) != ChatGroups.end())
		{
			// already exists
			return id;
		}

		ChatGroup newGroup;
		newGroup.Name = groupName;
		ChatGroups.insert_or_assign(id, std::move(newGroup));
		return id;
	}

	void DestroyChatGroup(size_t groupID)
	{
		std::lock_guard<std::mutex> lock(GroupsMutex);
		ChatGroups.erase(groupID);
	}

	void AddUserToGroup(ConnectedClient* client, size_t groupID)
	{
		auto chatClient = GetChatInfo(client);

		if (!chatClient)
			return;

		std::lock_guard<std::mutex> lock(GroupsMutex);
		auto itr = ChatGroups.find(groupID);
		if (itr == ChatGroups.end())
			return;
		itr->second.Members.push_back(chatClient);
	}

	void RemoveUserFromGroup(ConnectedClient* client, size_t groupID)
	{
		auto chatClient = GetChatInfo(client);

		if (!chatClient)
			return;

		std::lock_guard<std::mutex> lock(GroupsMutex);
		auto itr = ChatGroups.find(groupID);
		if (itr == ChatGroups.end())
			return;
		auto& members = itr->second.Members;
		auto memItr = std::find(members.begin(), members.end(), client);
		if (memItr != members.end())
		{
			members.erase(memItr);
		}

		chatClient->JoinedGroups.erase(groupID);
	}

	void SendServerMessage(std::string_view message, ConnectedClient* target)
	{
		if (target == nullptr)
		{
			for (auto [id, clientInfo] : ClientChatInfos)
			{
				clientInfo.Client->Send<Pack::ServerTextMessage>(message.data());
			}
			return;
		}

		target->Send<Pack::ServerTextMessage>(message.data());
	}

	void SendServerMessage(std::string_view message, size_t groupID)
	{
		auto itr = ChatGroups.find(groupID);
		if (itr == ChatGroups.end())
			return;

		for (auto member : itr->second.Members)
		{
			member->Client->Send<Pack::ServerTextMessage>(message.data());
		}
	}
}