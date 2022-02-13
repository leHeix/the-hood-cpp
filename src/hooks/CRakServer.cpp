#include "../main.hpp"

std::unique_ptr<net::CRakServer> net::RakServer;
static std::unordered_multimap<std::uint8_t, net::packet_receiver*> _packet_receivers;

net::packet_receiver::packet_receiver(net::raknet::PacketEnumeration packetid, const std::function<bool(std::uint16_t, BitStream*)>& fun)
	: call(fun)
{
	_packet_receivers.insert({ static_cast<uint8_t>(packetid), this });
}

namespace net
{
	CRakServer::CRakServer(void** plugin_data)
	{
		urmem::address_t addr;
		urmem::sig_scanner scanner;

		if (!scanner.init(reinterpret_cast<urmem::address_t>(*plugin_data)))
		{
			sampgdk::logprintf("[rakserver] Scanner initialization failed.");
			return;
		}

		using GetRakServer_t = uintptr_t(*)();
		_rakserver = reinterpret_cast<GetRakServer_t>(plugin_data[PLUGIN_DATA_RAKSERVER])();
		const auto vmt = urmem::pointer(_rakserver).field<urmem::address_t*>(0);

#ifdef _WIN32
		_Send_fun = vmt[7];
		_RPC_fun = vmt[32];
		_GetPlayerIdFromIndex_fun = vmt[58];
		_DeallocatePacket_fun = vmt[12];
		_Receive_fun = vmt[10];

		{
			urmem::unprotect_scope lk(reinterpret_cast<urmem::address_t>(&vmt[10]), sizeof(urmem::address_t));
			vmt[10] = reinterpret_cast<urmem::address_t>(&RakServer__Receive);
		}

		if (!scanner.find("\x8B\x44\x24\x04\x85\xC0\x75\x03\x0C\xFF\xC3\x8B\x48\x10\x8A\x01\x3C\xFF\x75\x03\x8A\x41\x05\xC3", "?????xxxxxxxxxxxx?xxxxxx", _GetPacketId_fun) || !_GetPacketId_fun)
#else
		_Send_fun = vmt[9];
		_RPC_fun = vmt[35];
		_GetPlayerIdFromIndex_fun = vmt[59];
		_DeallocatePacket_fun = vmt[13];
		_Receive_fun = vmt[11];

		{
			urmem::unprotect_scope lk(reinterpret_cast<urmem::address_t>(&vmt[11]), sizeof(urmem::address_t));
			vmt[11] = reinterpret_cast<urmem::address_t>(&RakServer__Receive);
		}

		if (!scanner.find("\x55\xB8\xFF\x00\x00\x00\x89\xE5\x8B\x55\x08\x85\xD2\x74\x0D\x8B\x52\x10\x0F\xB6\x02\x3C\xFF\x74\x07\x0F\xB6\xC0\x5D\xC3\x66\x90\x0F\xB6\x42\x05\x5D\xC3", "?????xxxxxxxxxxxxxxxxx?xxxxxxxxxxxxxxx", _GetPacketId_fun) || !_GetPacketId_fun)
#endif
		{
			sampgdk::logprintf("[RakNet] GetPacketID not found.");
			return;
		}
	}

	std::uint8_t CRakServer::GetPacketId(Packet* packet)
	{
		return urmem::call_function<urmem::calling_convention::cdeclcall, std::uint8_t>(_GetPacketId_fun, packet);
	}

	PlayerID CRakServer::GetPlayerIDFromIndex(int index) const
	{
		return urmem::call_function<urmem::calling_convention::thiscall, PlayerID>(_GetPlayerIdFromIndex_fun, _rakserver, index);
	}

	bool CRakServer::SendPacket(BitStream* bs, int index, PacketPriority priority, PacketReliability reliability) const
	{
		if (index == -1)
		{
			return urmem::call_function<urmem::calling_convention::thiscall, bool>(_Send_fun, _rakserver, bs, priority, reliability, 0, UNASSIGNED_PLAYER_ID, true);
		}
		return urmem::call_function<urmem::calling_convention::thiscall, bool>(_Send_fun, _rakserver, bs, priority, reliability, 0, GetPlayerIDFromIndex(index), 0);
	}

	bool CRakServer::SendPacket(BitStream* bs, PlayerID playerid, PacketPriority priority, PacketReliability reliability) const
	{
		return urmem::call_function<urmem::calling_convention::thiscall, bool>(_Send_fun, _rakserver, bs, priority, reliability, 0, playerid, (playerid == UNASSIGNED_PLAYER_ID));
	}

	bool CRakServer::SendRPC(BitStream* bs, unsigned char rpcid, int index, PacketPriority priority, PacketReliability reliability, unsigned ordering_channel, bool broadcast)
	{
		return urmem::call_function<urmem::calling_convention::thiscall, bool>(_RPC_fun, _rakserver, &rpcid, bs, priority, reliability, ordering_channel, GetPlayerIDFromIndex(index), broadcast, false);
	}

	bool CRakServer::SendRPC(BitStream* bs, unsigned char rpcid, PlayerID playerid, PacketPriority priority, PacketReliability reliability, unsigned ordering_channel, bool broadcast)
	{
		return urmem::call_function<urmem::calling_convention::thiscall, bool>(_RPC_fun, _rakserver, &rpcid, bs, priority, reliability, ordering_channel, playerid, broadcast, false);
	}

	void CRakServer::DeallocatePacket(Packet* packet)
	{
		return urmem::call_function<urmem::calling_convention::thiscall, void>(_DeallocatePacket_fun, _rakserver, packet);
	}

	Packet* CRakServer::Receive()
	{
		return urmem::call_function<urmem::calling_convention::thiscall, Packet*>(_Receive_fun, _rakserver);
	}

	Packet* FASTCALL RakServer__Receive(void* _this)
	{
		Packet* packet = RakServer->Receive();
		auto packetid = CRakServer::GetPacketId(packet);
		if (packetid == 0xFF)
			return packet;

		auto playerid = packet->playerIndex;
		if (playerid == static_cast<PlayerIndex>(-1))
			return packet;

		if (server::player_pool.Exists(playerid))
		{
			auto* player = server::player_pool[playerid];
			switch (packetid)
			{
				case net::raknet::ID_PLAYER_SYNC:
				{
					if (packet->length >= sizeof(stOnFootSyncData) + 1)
						return nullptr;

					player->LastUpdateTick() = std::chrono::steady_clock::now();

					stOnFootSyncData* data = reinterpret_cast<stOnFootSyncData*>(&packet->data[1]);
					player->Position().x = data->vecPos.X;
					player->Position().y = data->vecPos.Y;
					player->Position().z = data->vecPos.Z;
					GetPlayerFacingAngle(packet->playerIndex, &player->Position().w);

					break;
				}
				case net::raknet::ID_VEHICLE_SYNC:
				{
					if (packet->length >= sizeof(stVehicleSyncData) + 1)
						return nullptr;

					player->LastUpdateTick() = std::chrono::steady_clock::now();
					break;
				}
				case net::raknet::ID_PASSENGER_SYNC:
				{
					if (packet->length >= sizeof(stPassengerSyncData) + 1)
						return nullptr;

					player->LastUpdateTick() = std::chrono::steady_clock::now();
					break;
				}
				case net::raknet::ID_SPECTATOR_SYNC:
				{
					if (packet->length >= sizeof(stSpectatingSyncData) + 1)
						return nullptr;

					player->LastUpdateTick() = std::chrono::steady_clock::now();
					break;
				}
				case net::raknet::ID_AIM_SYNC:
				{
					if (packet->length >= sizeof(stAimSyncData) + 1)
						return nullptr;

					player->LastUpdateTick() = std::chrono::steady_clock::now();
					break;
				}
				case net::raknet::ID_TRAILER_SYNC:
				{
					if (packet->length >= sizeof(stTrailerSyncData) + 1)
						return nullptr;

					player->LastUpdateTick() = std::chrono::steady_clock::now();
					break;
				}
			}
		}

		BitStream bs{ &packet->data[0], packet->length, false };

		auto rg = _packet_receivers.equal_range(packetid);
		for (auto it = rg.first; it != rg.second; ++it)
		{
			if (!it->second->call(playerid, &bs))
			{
				RakServer->DeallocatePacket(packet);
				return packet;
			}

			bs.ResetReadPointer();
		}

		if (packet->data != bs.GetData())
		{
			RakServer->DeallocatePacket(packet);
			const size_t length = bs.GetNumberOfBytesUsed();
			packet = reinterpret_cast<Packet*>(std::malloc(sizeof(Packet) + length));
			packet->playerIndex = playerid;
			packet->playerId = RakServer->GetPlayerIDFromIndex(playerid);
			packet->length = length;
			packet->bitSize = bs.GetNumberOfBitsUsed();
			packet->data = reinterpret_cast<unsigned char*>(&packet[1]);
			std::memcpy(packet->data, bs.GetData(), length);
			packet->deleteData = false;
		}

		return packet;
	}
};
