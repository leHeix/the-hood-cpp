#pragma once

namespace net
{	
	struct packet_receiver
	{
		std::function<bool(std::uint16_t, BitStream*)> call;
		packet_receiver(raknet::PacketEnumeration packetid, const std::function<bool(std::uint16_t, BitStream*)>& fun);
	};

	class CRakServer
	{
	private:
		urmem::address_t _rakserver;
		urmem::address_t _Send_fun;
		urmem::address_t _RPC_fun;
		urmem::address_t _GetPlayerIdFromIndex_fun;
		urmem::address_t _DeallocatePacket_fun;
		urmem::address_t _Receive_fun;
		inline static urmem::address_t _GetPacketId_fun;

	public:
		CRakServer(void** plugin_data);
		~CRakServer() = default;

		PlayerID GetPlayerIDFromIndex(int index) const;
		static std::uint8_t GetPacketId(Packet* packet);

		bool SendPacket(BitStream* bs, int index = -1, PacketPriority priority = LOW_PRIORITY, PacketReliability reliability = RELIABLE) const;
		bool SendPacket(BitStream* bs, PlayerID playerid = UNASSIGNED_PLAYER_ID, PacketPriority priority = LOW_PRIORITY, PacketReliability reliability = RELIABLE) const;
		bool SendRPC(BitStream* bs, unsigned char rpcid, int index, PacketPriority priority = HIGH_PRIORITY, PacketReliability reliability = RELIABLE, unsigned ordering_channel = 0, bool broadcast = false);
		bool SendRPC(BitStream* bs, unsigned char rpcid, PlayerID playerid, PacketPriority priority = HIGH_PRIORITY, PacketReliability reliability = RELIABLE, unsigned ordering_channel = 0, bool broadcast = false);
		void DeallocatePacket(Packet* packet);
		Packet* Receive();
	};

	extern std::unique_ptr<CRakServer> RakServer;

	Packet* FASTCALL RakServer__Receive(void* _this);
};
