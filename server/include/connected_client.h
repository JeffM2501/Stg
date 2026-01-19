#pragma once

#include "enet.h"

#include "Messages.h"
#include "atomic_queue.h"
#include "events.h"
#include "lifetime_token.h"

#include <memory>
#include <mutex>
#include <atomic>

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

    std::atomic<ClientState> State = ClientState::Unknown;

    std::mutex ClientLock;

public:
    Tokens::LifetimeTokenPtr GetToken() { return TokenSource.GetToken(); }

    ENetPeer* Peer = nullptr;

	std::atomic<uint32_t> CurrentRoomID = 0;

    ConnectedClient(ENetPeer* peer) : Peer(peer) {}
    AtomicQueue<ENetPacket*> InboundPackets;
    AtomicQueue<std::shared_ptr<MessageBuffer>> OutboundPackets;

    Events::EventSource<ConnectedClient> OnStateChanged;
    Events::EventSource<ConnectedClient> OnDisconnectd;

    template<class T, typename... Args>
    void Send(Args&&... args)
    {
        std::shared_ptr<MessageBuffer> message = CreateMessage<T>(std::forward<Args>(args)...);
        OutboundPackets.Push(message);
    }

    void Send(std::shared_ptr<MessageBuffer>& message)
    {
        OutboundPackets.Push(message);
    }

    template<class T, typename... Args>
    std::shared_ptr<T> CreateMessage(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    void SetState(ClientState state);
    ClientState GetState();
};