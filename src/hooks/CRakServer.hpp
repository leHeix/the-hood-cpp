#pragma once

namespace net
{
    class CRakServer
    {
    private:
        urmem::address_t _rakserver;
        urmem::address_t _Send_fun;
        urmem::address_t _RPC_fun;
        urmem::address_t _GetPlayerIdFromIndex_fun;
        inline static urmem::hook _GetPacketId_hook;

    public:
        CRakServer(void** plugin_data);
        ~CRakServer() = default;

        PlayerID GetPlayerIDFromIndex(int index) const;
        static uint8_t GetPacketID(Packet* packet);

        bool SendPacket(BitStream* bs, int index = -1, PacketPriority priority = LOW_PRIORITY, PacketReliability reliability = RELIABLE) const;
        bool SendPacket(BitStream* bs, PlayerID playerid = UNASSIGNED_PLAYER_ID, PacketPriority priority = LOW_PRIORITY, PacketReliability reliability = RELIABLE) const;
        bool SendRPC(BitStream* bs, unsigned char rpcid, int index, PacketPriority priority = HIGH_PRIORITY, PacketReliability reliability = RELIABLE, unsigned ordering_channel = 0, bool broadcast = false);
        bool SendRPC(BitStream* bs, unsigned char rpcid, PlayerID playerid, PacketPriority priority = HIGH_PRIORITY, PacketReliability reliability = RELIABLE, unsigned ordering_channel = 0, bool broadcast = false);
    };

    extern std::unique_ptr<CRakServer> RakServer;
};