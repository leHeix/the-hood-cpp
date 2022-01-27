#include "../main.hpp"

std::unique_ptr<net::CRakServer> net::RakServer;

namespace net
{
	CRakServer::CRakServer(void** plugin_data)
	{
		urmem::address_t addr;
		urmem::sig_scanner scanner;

		if (!scanner.init(reinterpret_cast<urmem::address_t>(*plugin_data)))
		{
			sampgdk::logprintf("[RakNet] Scanner initialization failed.");
			return;
		}

		_rakserver = reinterpret_cast<int(*)(void)>(plugin_data[PLUGIN_DATA_RAKSERVER])();
		const auto vmt = urmem::pointer(_rakserver).field<urmem::address_t*>(0);

#ifdef _WIN32
		_Send_fun = vmt[7];
		_RPC_fun = vmt[32];
		_GetPlayerIdFromIndex_fun = vmt[58];

		if (!scanner.find("\x8B\x44\x24\x04\x85\xC0\x75\x03\x0C\xFF\xC3", "xxxxxxx???x", addr) || !addr)
#else
		_Send_fun = vmt[9];
		_RPC_fun = vmt[35];
		_GetPlayerIdFromIndex_fun = vmt[59];

		if (!scanner.find("\x55\xB8\x00\x00\x00\x00\x89\xE5\x8B\x55\x00\x85\xD2", "xx????xxxx?xx", addr) || !addr)
#endif
		{
			sampgdk::logprintf("[RakNet] GetPacketID not found.");
			return;
		}

		_GetPacketId_hook.install(addr, urmem::get_func_addr(&GetPacketID));
	}

	uint8_t CRakServer::GetPacketID(Packet* packet)
	{
		if (!packet || !packet->data || packet->length == 0)
			return 0xFF;

		int packet_id = packet->data[0];

		if (packet_id < 0 || packet_id > 255)
			return 0xFF;

		uint16_t packetid;

		std::unique_ptr<BitStream> bs = std::make_unique<BitStream>(&packet->data[0], (packet->length - 1), false);
		bs->Read<uint16_t>(packetid);

		if (server::player_pool.Exists(packet->playerIndex))
		{
			auto* player = server::player_pool[packet->playerIndex];

			switch (packetid)
			{
				case net::raknet::ID_PLAYER_SYNC:
				{
					if (packet->length != sizeof(stOnFootSyncData) + 1)
						return 0xFF;

					stOnFootSyncData* data = (stOnFootSyncData*)&packet->data[1];
					player->Position().x = data->vecPos.X;
					player->Position().y = data->vecPos.Y;
					player->Position().z = data->vecPos.Z;
					GetPlayerFacingAngle(packet->playerIndex, &player->Position().w);

					/*
					glm::quat q;
					q.w = data->fQuaternion[0];
					q.x = data->fQuaternion[1];
					q.y = data->fQuaternion[2];
					q.z = data->fQuaternion[3];
					glm::vec3 e = glm::eulerAngles(q);
					sampgdk::logprintf("roll: %f - pitch: %f - yaw: %f", glm::roll(q), glm::pitch(q), glm::yaw(q));
					sampgdk::logprintf("vector: %f - %f - %f", e.x, e.y, e.z);
					e = glm::degrees(e);
					sampgdk::logprintf("vector deg: %f - %f - %f", e.x, e.y, e.z);
					sampgdk::logprintf("angle gpfa: %f", player->Position().w);
					*/
					break;
				}
			}
		}

		return _GetPacketId_hook.call<urmem::calling_convention::cdeclcall, uint8_t>(packet);
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
};