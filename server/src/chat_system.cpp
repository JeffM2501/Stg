#include "chat_system.h"

#include "client_database.h"
#include "message_route_ids.h"
#include "message_router.h"


#include "messages/chat_group_messages.h"
#include "messages/chat_messages.h"

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
		std::set<uint32_t> JoinedGroups;

		std::set<uint32_t> KnownUsers;
	};

	std::unordered_map<uint32_t, ClientChatInfo> ClientChatInfos;

	struct ChatGroup
	{
	public:
		std::string Name;
		std::vector<ClientChatInfo*> Members;
	};

	std::mutex GroupsMutex;
	std::unordered_map<uint32_t, ChatGroup> ChatGroups;

    void InformUserOfPeer(ClientChatInfo* user, ClientChatInfo* peer)
    {
        user->Client->Send<ChatGroupMessages::ServerAddChatUser>(peer->Client->Peer->connectID, peer->Name);
        user->KnownUsers.insert(peer->Client->Peer->connectID);
    }

	ClientChatInfo* GetChatInfo(ConnectedClient* client)
	{
		auto itr = ClientChatInfos.find(client->Peer->connectID);
		if (itr == ClientChatInfos.end())
			return nullptr;
		return &itr->second;
	}

    ClientChatInfo* GetChatInfo(uint32_t clientID)
    {
        auto itr = ClientChatInfos.find(clientID);
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

	void ProcessChatMessage(ConnectedClient& sender, ChatMessages::ClientTextMessage* message)
	{
		// route to everyone else in the channel
        auto* chatClient = GetChatInfo(&sender);
        if (!chatClient)
            return;

		if (message->GroupID() == 0)
		{
			if (message->TargetId() == 0)
			{
                return; // LOG? who his this to?
			}

			// private message

			auto* targetInfo = GetChatInfo(message->TargetId());

			if (!chatClient->KnownUsers.contains(message->TargetId()))
			{
				if (!targetInfo)
					return; // LOG?
				InformUserOfPeer(chatClient, targetInfo);
			}

			SendServerMessage(message->Message(), targetInfo->Client, chatClient->Client->Peer->connectID);
		}

        // group message
		if (!chatClient->JoinedGroups.contains(message->GroupID()))
			return; // LOG?
		
        SendServerMessage(message->Message(), message->GroupID(), chatClient->Client->Peer->connectID);
	}

	void RouteMessage(ConnectedClient* client, std::unique_ptr<MessageBuffer> buffer)
	{
		if (buffer == nullptr)
			return;

		switch (buffer->MessageTypeId)
		{
		case MessageIDS::ClientTextMessage:
			ProcessChatMessage(*client, static_cast<ChatMessages::ClientTextMessage*>(buffer.get()));
			break;
		}
	}

	void NewConnection(void* sender, ConnectedClient* client)
	{
		ClientChatInfo newClient;
		newClient.Client = client;
		newClient.Name = GenerateGuestName();
		ClientChatInfos.insert_or_assign(client->Peer->connectID, newClient);

		// tell them the name
		client->Send<ChatGroupMessages::ServerAddChatUser>(newClient.Client->Peer->connectID, newClient.Name);

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
				other->Send<ChatGroupMessages::ServerRemoveChatUser>(id);
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

	static uint32_t LastGroupId = 0;

	uint32_t CreateChatGroup(std::string_view groupName)
	{
		LastGroupId++;

		std::lock_guard<std::mutex> lock(GroupsMutex);

		ChatGroup newGroup;
		newGroup.Name = groupName;
		ChatGroups.insert_or_assign(LastGroupId, std::move(newGroup));
		return LastGroupId;
	}

	void DestroyChatGroup(uint32_t groupID)
	{
		std::lock_guard<std::mutex> lock(GroupsMutex);
		ChatGroups.erase(groupID);
	}

	void AddUserToGroup(ConnectedClient* client, uint32_t groupID)
	{
		auto chatClient = GetChatInfo(client);

		if (!chatClient)
			return;

		std::lock_guard<std::mutex> lock(GroupsMutex);
		auto itr = ChatGroups.find(groupID);
		if (itr == ChatGroups.end())
			return;

		auto& group = itr->second;
		group.Members.push_back(chatClient);

		// tell them about the group
		client->Send<ChatGroupMessages::ServerSetChatGroup>(groupID, group.Name);
		
		for (auto& member : group.Members)
		{
			// tell them about everyone in the chat
			client->Send<ChatGroupMessages::ServerAddChatUser>(member->Client->Peer->connectID, member->Name);
			chatClient->KnownUsers.insert(member->Client->Peer->connectID);

			// tell everyone in the chat about them
			member->Client->Send<ChatGroupMessages::ServerAddChatUser>(client->Peer->connectID, chatClient->Name);
			member->KnownUsers.insert(chatClient->Client->Peer->connectID);
		}
	}

	void RemoveUserFromGroup(ConnectedClient* client, uint32_t groupID)
	{
		auto* chatClient = GetChatInfo(client);

		if (!chatClient)
			return;

		std::lock_guard<std::mutex> lock(GroupsMutex);
		auto itr = ChatGroups.find(groupID);
		if (itr == ChatGroups.end())
			return;
		auto& members = itr->second.Members;

		for (auto member : members)
		{
			member->Client->Send<ChatGroupMessages::ServerRemoveChatUser>(client->Peer->connectID, groupID);
		}

		auto memItr = std::find(members.begin(), members.end(), chatClient);
		if (memItr != members.end())
		{
			members.erase(memItr);
		}

		chatClient->JoinedGroups.erase(groupID);
	}

    void SendServerMessage(std::string_view message, ConnectedClient* target, uint32_t senderId)
	{
		if (target == nullptr) // send to ALL
		{
			for (auto [id, clientInfo] : ClientChatInfos)
			{
				clientInfo.Client->Send<ChatMessages::ServerTextMessage>(message.data(), senderId);
			}
			return;
		}

		target->Send<ChatMessages::ServerTextMessage>(message.data(), senderId);
	}

	void SendServerMessage(std::string_view message, uint32_t groupID, uint32_t senderId)
	{
		ClientChatInfo* sender = nullptr;
		
		if (senderId != 0)
		{
			sender = GetChatInfo(senderId);
			if (!sender)
				return;
		}

		auto itr = ChatGroups.find(groupID);
		if (itr == ChatGroups.end())
			return;

		for (auto member : itr->second.Members)
		{
			if (sender && !member->KnownUsers.contains(senderId))
                InformUserOfPeer(member, sender);

			member->Client->Send<ChatMessages::ServerTextMessage>(message.data(), senderId, groupID);
		}
	}
}