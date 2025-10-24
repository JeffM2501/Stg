#pragma once

#include "enet.h"

#include "Events.h"
#include "LifetimeToken.h"

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

	ClientState State = ClientState::Unknown;

	AtomicQueue<ENetPacket*> InboundPackets;
	AtomicQueue<ENetPacket*> OutboundPackets;

	Events::EventSource<ConnectedClient> OnStateChanged;
	Events::EventSource<ConnectedClient> OnDisconnectd;
};
