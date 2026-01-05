#pragma once
#include <string_view>
#include <span>
#include <string>

#include "events.h"
#include "enet.h"

#include <unordered_map>

namespace Connection
{
	class MessageHandler
	{
	public:
		virtual void Process(ENetPacket* packet) = 0;
	};

	template<class T>
	class TypedMessageHandler : public MessageHandler
	{
	public:
		using MessageType = T;
		using MessageProcessFunc = std::function<void(const T&)>;

		MessageProcessFunc ProcessFunc = nullptr;
		TypedMessageHandler(MessageProcessFunc func = nullptr) : ProcessFunc(func) {}
		void Process(ENetPacket* packet) override
		{
			T message(packet);
			ProcessFunc(message);
		}
	};

	extern std::unordered_map<size_t, std::unique_ptr<MessageHandler>> MessageHandlers;

	template<class T>
	TypedMessageHandler<T>& RegisterHandler()
	{
		auto handler = std::make_unique<TypedMessageHandler<T>>();
		auto* ref = handler.get();
		MessageHandlers.insert_or_assign(T::TypeId(), std::move(handler));
		return *ref;
	}

	void Init();
	void Cleanup();

	bool Connect(std::string_view host = "127.0.0.1");

	bool IsConnected();

	void ServiceNetwork();

	uint32_t GetClientId();
	std::span<std::string> GetServerChat();

	extern Events::EventSource<uint32_t> OnConnected;
	extern Events::EventSource<uint32_t> OnConnectionComplete;
}