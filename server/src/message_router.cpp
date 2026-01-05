#include "message_router.h"

namespace MessageRouter
{
	using RouteMap = std::unordered_map<int, std::function<void(ConnectedClient* client, std::unique_ptr<MessageUnpackBuffer> buffer)>>;

	static RouteMap RouteHandlers;
	void PacketReceive(ENetPeer* peer, ENetPacket* packet)
	{
		ConnectedClient* client = ClientDB::GetClient(peer);
		if (client == nullptr)
		{
			enet_packet_destroy(packet);
			return;
		}
		std::unique_ptr<MessageUnpackBuffer> buffer = MessageUnpackFactories::Unpack(packet);

		if (buffer == nullptr)
		{
			enet_packet_destroy(packet);
			return;
		}

		uint64_t messageTypeID = MessageUnpackBuffer::GetMessageTypeID(packet);
		if (messageTypeID == MessageUnpackBuffer::InvalidMessageTypeID)
		{
			enet_packet_destroy(packet);
			return;
		}
		RouteMap::iterator itr = RouteHandlers.find(static_cast<int>(buffer->GetProcessingChannel()));
		if (itr == RouteHandlers.end())
		{
			enet_packet_destroy(packet);
			return;
		}
		itr->second(client, std::move(buffer));
	}

	void RegisterRouteHandler(int handlerId, std::function<void(ConnectedClient* client, std::unique_ptr<MessageUnpackBuffer> buffer)> func)
	{
		RouteHandlers.insert_or_assign(handlerId, func);
	}
}