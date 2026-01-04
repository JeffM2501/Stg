#pragma once

#include "enet.h"

#include "Messages.h"
#include "AtomicQueue.h"
#include "Events.h"
#include "LifetimeToken.h"

#include <memory>

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
private:
	Tokens::TokenSource TokenSource;
public:
	Tokens::LifetimeTokenPtr GetToken() { return TokenSource.GetToken(); }

    ENetPeer* Peer = nullptr;

	ClientState State = ClientState::Unknown;

	AtomicQueue<ENetPacket*> InboundPackets;
	AtomicQueue<std::shared_ptr<MessagePackBuffer>> OutboundPackets;

	Events::EventSource<ConnectedClient> OnStateChanged;
	Events::EventSource<ConnectedClient> OnDisconnectd;

    ConnectedClient(ENetPeer* peer = nullptr) : Peer(peer) {}
};
