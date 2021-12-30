/*

	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, BlastHack

	mod_sa is available from https://github.com/BlastHackNet/mod_s0beit_sa/

	mod_sa is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	mod_sa is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with mod_sa.  If not, see <http://www.gnu.org/licenses/>.

*/
#pragma once

#include "BitStream.h"

#define RAKNET_MAX_PACKET	256

typedef unsigned int RakNetTime;
typedef long long RakNetTimeNS;

enum eIncomingRPC
{
	IRPC_SetName = 11,
	IRPC_SetPosition = 12,
	IRPC_SetPositionFindZ = 13,
	IRPC_SetHealth = 14,
	IRPC_ToggleControllable = 15,
	IRPC_PlayGameSound = 16,
	IRPC_SetWorldBounds = 17,
	IRPC_GiveMoney = 18,
	IRPC_SetFacingAngle = 19,
	IRPC_ResetMoney = 20,
	IRPC_ResetWeapons = 21,
	IRPC_GiveWeapon = 22,
	IRPC_ClickPlayer = 23,
	IRPC_SetVehicleParamsEx = 24,
	IRPC_EnterVehicle = 26,
	IRPC_SelectObject = 27,
	IRPC_CancelEdit = 28,
	IRPC_SetTimeEx = 29,
	IRPC_ToggleClock = 30,
	IRPC_WorldPlayerAdd = 32,
	IRPC_SetShopName = 33,
	IRPC_SetPlayerSkillLevel = 34,
	IRPC_SetDrunkLevel = 35,
	IRPC_Create3DTextLabel = 36,
	IRPC_DisableCheckpoint = 37,
	IRPC_SetRaceCheckpoint = 38,
	IRPC_DisableRaceCheckpoint = 39,
	IRPC_GameModeRestart = 40,
	IRPC_PlayAudioStream = 41,
	IRPC_StopAudioStream = 42,
	IRPC_RemoveBuilding = 43,
	IRPC_CreateObject = 44,
	IRPC_SetObjectPos = 45,
	IRPC_SetObjectRot = 46,
	IRPC_DestroyObject = 47,
	IRPC_SendDeathMessage = 55,
	IRPC_SetMapIcon = 56,
	IRPC_RemoveVehicleComponent = 57,
	IRPC_TextLabelStreamedOut = 58,
	IRPC_ChatBubble = 59,
	IRPC_PlayerUpdate = 60,
	IRPC_ShowDialog = 61,
	IRPC_DestroyPickup = 63,
	IRPC_LinkVehicleToInterior = 65,
	IRPC_SetArmour = 66,
	IRPC_SetArmedWeapon = 67,
	IRPC_SetSpawnInfo = 68,
	IRPC_SetTeam = 69,
	IRPC_PutInVehicle = 70,
	IRPC_RemoveFromVehicle = 71,
	IRPC_SetPlayerColor = 72,
	IRPC_GameText = 73,
	IRPC_ForceClassSelection = 74,
	IRPC_AttachObjectToPlayer = 75,
	IRPC_InitMenu = 76,
	IRPC_ShowMenu = 77,
	IRPC_HideMenu = 78,
	IRPC_CreateExplosion = 79,
	IRPC_ShowNameTag = 80,
	IRPC_AttachCameraToObject = 81,
	IRPC_InterpolateCamera = 82,
	IRPC_SelectTextDraw = 83,
	IRPC_SetObjectMaterial = 84,
	IRPC_StopFlashGangZone = 85,
	IRPC_ApplyAnimation = 86,
	IRPC_ClearAnimations = 87,
	IRPC_SetSpecialAction = 88,
	IRPC_SetFightingStyle = 89,
	IRPC_SetVelocity = 90,
	IRPC_SetVehicleVelocity = 91,
	IRPC_ClientMessage = 93,
	IRPC_WorldTime = 94,
	IRPC_CreatePickup = 95,
	IRPC_ScmEvent = 96,
	IRPC_DestroyWeaponPickup = 97,
	IRPC_MoveObject = 99,
	IRPC_Chat = 101,
	IRPC_SvrStats = 102,
	IRPC_ClientCheck = 103,
	IRPC_EnableStuntBonus = 104,
	IRPC_EditTextDraw = 105,
	IRPC_UpdateVehicleDamageStatus = 106,
	IRPC_SetCheckpoint = 107,
	IRPC_ShowGangZone = 108,
	IRPC_PlayCrimeReport = 112,
	IRPC_SetPlayerAttachedObject = 113,
	IRPC_EditAttachedObject = 116,
	IRPC_EditObject = 117,
	IRPC_HideGangZone = 120,
	IRPC_FlashGangZone = 121,
	IRPC_StopObject = 122,
	IRPC_SetNumberPlate = 123,
	IRPC_ToggleSpectating = 124,
	IRPC_SpectatePlayer = 126,
	IRPC_SpectateVehicle = 127,
	IRPC_RequestClass = 128,
	IRPC_SpawnPlayer = 129,
	IRPC_ConnectionRejected = 130,
	IRPC_SetWantedLevel = 133,
	IRPC_ShowTextDraw = 134,
	IRPC_HideTextDraw = 135,
	IRPC_ServerJoin = 137,
	IRPC_ServerQuit = 138,
	IRPC_InitGame = 139,
	IRPC_DisableMapIcon = 144,
	IRPC_SetAmmo = 145,
	IRPC_SetGravity = 146,
	IRPC_SetVehicleHealth = 147,
	IRPC_AttachTrailerToVehicle = 148,
	IRPC_DetachTrailerFromVehicle = 149,
	IRPC_PickedUpWeapon = 151,
	IRPC_Weather = 152,
	IRPC_SetPlayerSkin = 153,
	IRPC_ExitVehicle = 154,
	IRPC_UpdateScoresPingIPS = 155,
	IRPC_SetInterior = 156,
	IRPC_SetCameraPos = 157,
	IRPC_SetCameraLookAt = 158,
	IRPC_SetVehiclePos = 159,
	IRPC_SetVehicleZAngle = 160,
	IRPC_SetVehicleParams = 161,
	IRPC_SetCameraBehindPlayer = 162,
	IRPC_WorldPlayerRemove = 163,
	IRPC_WorldVehicleAdd = 164,
	IRPC_WorldVehicleRemove = 165,
	IRPC_Death = 166,
	IRPC_DisableVehicleCollision = 167,
	IRPC_ObjectNoCameraCol = 169,
	IRPC_EnablePlayerCameraTarget = 170,
	IRPC_WorldActorAdd = 171,
	IRPC_WorldActorRemove = 172,
	IRPC_ApplyActorAnimation = 173,
	IRPC_ClearActorAnimations = 174,
	IRPC_SetActorFacingAngle = 175,
	IRPC_SetActorPos = 176,
	IRPC_SetActorHealth = 178
};

enum eOutgoingRPC
{
	ORPC_ClickPlayer = 23,
	ORPC_ClientJoin = 25,
	ORPC_EnterVehicle = 26,
	ORPC_SelectObject = 27,
	ORPC_ScriptCash = 31,
	ORPC_ServerCommand = 50,
	ORPC_Spawn = 52,
	ORPC_Death = 53,
	ORPC_NPCJoin = 54,
	ORPC_DialogResponse = 62,
	ORPC_ClickTextDraw = 83,
	ORPC_ScmEvent = 96,
	ORPC_PickedUpWeapon = 97,
	ORPC_Chat = 101,
	ORPC_NetStats = 102,
	ORPC_ClientCheck = 103,
	ORPC_DamageVehicle = 106,
	ORPC_GiveTakeDamage = 115,
	ORPC_EditAttachedObject = 116,
	ORPC_EditObject = 117,
	ORPC_SetInteriorId = 118,
	ORPC_MapMarker = 119,
	ORPC_RequestClass = 128,
	ORPC_RequestSpawn = 129,
	ORPC_PickedUpPickUp = 131,
	ORPC_MenuSelect = 132,
	ORPC_VehicleDestroyed = 136,
	ORPC_MenuQuit = 140,
	ORPC_ExitVehicle = 154,
	ORPC_UpdateScorePingIPs = 155,
	ORPC_SetCameraTarget = 168,
	ORPC_GiveDamageActor = 177
};

enum PacketEnumeration
{
	ID_INTERNAL_PING = 6,
	ID_PING,
	ID_PING_OPEN_CONNECTIONS,
	ID_CONNECTED_PONG,
	ID_REQUEST_STATIC_DATA,
	ID_CONNECTION_REQUEST,
	ID_AUTH_KEY,
	ID_BROADCAST_PINGS = 15,
	ID_SECURED_CONNECTION_RESPONSE,
	ID_SECURED_CONNECTION_CONFIRMATION,
	ID_MAPPING,
	ID_SET_RANDOM_NUMBER_SEED = 21,
	ID_RPC,
	ID_REPLY,
	ID_DETECT_LOST_CONNECTIONS,
	ID_OPEN_CONNECTION_REQUEST,
	ID_OPEN_CONNECTION_REPLY,
	ID_RSA_PUBLIC_KEY_MISMATCH = 28,
	ID_CONNECTION_ATTEMPT_FAILED,
	ID_NEW_INCOMING_CONNECTION,
	ID_NO_FREE_INCOMING_CONNECTIONS,
	ID_DISCONNECTION_NOTIFICATION,
	ID_CONNECTION_LOST,
	ID_CONNECTION_REQUEST_ACCEPTED,
	ID_INITIALIZE_ENCRYPTION,
	ID_CONNECTION_BANNED,
	ID_INVALID_PASSWORD,
	ID_MODIFIED_PACKET,
	ID_PONG,
	ID_TIMESTAMP,
	ID_RECEIVED_STATIC_DATA,
	ID_REMOTE_DISCONNECTION_NOTIFICATION,
	ID_REMOTE_CONNECTION_LOST,
	ID_REMOTE_NEW_INCOMING_CONNECTION,
	ID_REMOTE_EXISTING_CONNECTION,
	ID_REMOTE_STATIC_DATA,
	ID_ADVERTISE_SYSTEM = 56,

	ID_VEHICLE_SYNC = 200,
	ID_AIM_SYNC = 203,
	ID_BULLET_SYNC = 206,
	ID_PLAYER_SYNC = 207,
	ID_MARKERS_SYNC,
	ID_UNOCCUPIED_SYNC,
	ID_TRAILER_SYNC,
	ID_PASSENGER_SYNC,
	ID_SPECTATOR_SYNC,
	ID_RCON_COMMAND,
	ID_RCON_RESPONCE,
	ID_WEAPONS_UPDATE,
	ID_STATS_UPDATE,
};

/// These enumerations are used to describe when packets are delivered.
enum PacketPriority
{
	SYSTEM_PRIORITY,   /// \internal Used by RakNet to send above-high priority messages.
	HIGH_PRIORITY,   /// High priority messages are send before medium priority messages.
	MEDIUM_PRIORITY,   /// Medium priority messages are send before low priority messages.
	LOW_PRIORITY,   /// Low priority messages are only sent when no other messages are waiting.
	NUMBER_OF_PRIORITIES
};

/// These enumerations are used to describe how packets are delivered.
/// \note  Note to self: I write this with 3 bits in the stream.  If I add more remember to change that
enum PacketReliability
{
	UNRELIABLE = 6,   /// Same as regular UDP, except that it will also discard duplicate datagrams.  RakNet adds (6 to 17) + 21 bits of overhead, 16 of which is used to detect duplicate packets and 6 to 17 of which is used for message length.
	UNRELIABLE_SEQUENCED,  /// Regular UDP with a sequence counter.  Out of order messages will be discarded.  This adds an additional 13 bits on top what is used for UNRELIABLE.
	RELIABLE,   /// The message is sent reliably, but not necessarily in any order.  Same overhead as UNRELIABLE.
	RELIABLE_ORDERED,   /// This message is reliable and will arrive in the order you sent it.  Messages will be delayed while waiting for out of order messages.  Same overhead as UNRELIABLE_SEQUENCED.
	RELIABLE_SEQUENCED /// This message is reliable and will arrive in the sequence you sent it.  Out or order messages will be dropped.  Same overhead as UNRELIABLE_SEQUENCED.
};

typedef unsigned short PlayerIndex;

#pragma pack(push, 1)
struct PlayerID
{
	unsigned int binaryAddress;
	unsigned short port;

	bool operator==(const PlayerID& other) const
	{
		return (this->binaryAddress == other.binaryAddress && this->port == other.port);
	}
};

struct NetworkID
{
	bool peerToPeer;
	PlayerID playerId;
	unsigned short localSystemId;
};

/// This represents a user message from another system.
struct Packet
{
	/// Server only - this is the index into the player array that this playerId maps to
	PlayerIndex playerIndex;

	/// The system that send this packet.
	PlayerID playerId;

	/// The length of the data in bytes
	/// \deprecated You should use bitSize.
	unsigned int length;

	/// The length of the data in bits
	unsigned int bitSize;

	/// The data from the sender
	unsigned char* data;

	/// @internal
	/// Indicates whether to delete the data, or to simply delete the packet.
	bool deleteData;
};

/// All RPC functions have the same parameter list - this structure.
struct RPCParameters
{
	/// The data from the remote system
	unsigned char* input;

	/// How many bits long \a input is
	unsigned int numberOfBitsOfData;

	/// Which system called this RPC
	PlayerID sender;

	/// Which instance of RakPeer (or a derived RakServer or RakClient) got this call
	void* recipient;

	/// You can return values from RPC calls by writing them to this BitStream.
	/// This is only sent back if the RPC call originally passed a BitStream to receive the reply.
	/// If you do so and your send is reliable, it will block until you get a reply or you get disconnected from the system you are sending to, whichever is first.
	/// If your send is not reliable, it will block for triple the ping time, or until you are disconnected, or you get a reply, whichever is first.
	BitStream* replyToSender;
};

struct RPCNode
{
	uint8_t uniqueIdentifier;
	void(*staticFunctionPointer) (RPCParameters* rpcParms);
};

/// Store Statistics information related to network usage 
struct RakNetStatisticsStruct
{
	///  Number of Messages in the send Buffer (high, medium, low priority)
	unsigned messageSendBuffer[NUMBER_OF_PRIORITIES];
	///  Number of messages sent (high, medium, low priority)
	unsigned messagesSent[NUMBER_OF_PRIORITIES];
	///  Number of data bits used for user messages
	unsigned messageDataBitsSent[NUMBER_OF_PRIORITIES];
	///  Number of total bits used for user messages, including headers
	unsigned messageTotalBitsSent[NUMBER_OF_PRIORITIES];

	///  Number of packets sent containing only acknowledgements
	unsigned packetsContainingOnlyAcknowlegements;
	///  Number of acknowledgements sent
	unsigned acknowlegementsSent;
	///  Number of acknowledgements waiting to be sent
	unsigned acknowlegementsPending;
	///  Number of acknowledgements bits sent
	unsigned acknowlegementBitsSent;

	///  Number of packets containing only acknowledgements and resends
	unsigned packetsContainingOnlyAcknowlegementsAndResends;

	///  Number of messages resent
	unsigned messageResends;
	///  Number of bits resent of actual data
	unsigned messageDataBitsResent;
	///  Total number of bits resent, including headers
	unsigned messagesTotalBitsResent;
	///  Number of messages waiting for ack (// TODO - rename this)
	unsigned messagesOnResendQueue;

	///  Number of messages not split for sending
	unsigned numberOfUnsplitMessages;
	///  Number of messages split for sending
	unsigned numberOfSplitMessages;
	///  Total number of splits done for sending
	unsigned totalSplits;

	///  Total packets sent
	unsigned packetsSent;

	///  Number of bits added by encryption
	unsigned encryptionBitsSent;
	///  total bits sent
	unsigned totalBitsSent;

	///  Number of sequenced messages arrived out of order
	unsigned sequencedMessagesOutOfOrder;
	///  Number of sequenced messages arrived in order
	unsigned sequencedMessagesInOrder;

	///  Number of ordered messages arrived out of order
	unsigned orderedMessagesOutOfOrder;
	///  Number of ordered messages arrived in order
	unsigned orderedMessagesInOrder;

	///  Packets with a good CRC received
	unsigned packetsReceived;
	///  Packets with a bad CRC received
	unsigned packetsWithBadCRCReceived;
	///  Bits with a good CRC received
	unsigned bitsReceived;
	///  Bits with a bad CRC received
	unsigned bitsWithBadCRCReceived;
	///  Number of acknowledgement messages received for packets we are resending
	unsigned acknowlegementsReceived;
	///  Number of acknowledgement messages received for packets we are not resending
	unsigned duplicateAcknowlegementsReceived;
	///  Number of data messages (anything other than an ack) received that are valid and not duplicate
	unsigned messagesReceived;
	///  Number of data messages (anything other than an ack) received that are invalid
	unsigned invalidMessagesReceived;
	///  Number of data messages (anything other than an ack) received that are duplicate
	unsigned duplicateMessagesReceived;
	///  Number of messages waiting for reassembly
	unsigned messagesWaitingForReassembly;
	///  Number of messages in reliability output queue
	unsigned internalOutputQueueSize;
	///  Current bits per second
	double bitsPerSecond;
	///  connection start time
	RakNetTime connectionStartTime;
};
#pragma pack(pop)

class RakClientInterface
{
public:
	virtual ~RakClientInterface() {};
	virtual bool Connect(const char* host, unsigned short serverPort, unsigned short clientPort, unsigned int depreciated, int threadSleepTimer);
	virtual void Disconnect(unsigned int blockDuration, unsigned char orderingChannel = 0);
	virtual void InitializeSecurity(const char* privKeyP, const char* privKeyQ);
	virtual void SetPassword(const char* _password);
	virtual bool HasPassword(void) const;
	virtual bool Send(const char* data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel);
	virtual bool Send(BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel);
	virtual Packet* Receive(void);
	virtual void DeallocatePacket(Packet* packet);
	virtual void PingServer(void);
	virtual void PingServer(const char* host, unsigned short serverPort, unsigned short clientPort, bool onlyReplyOnAcceptingConnections);
	virtual int GetAveragePing(void);
	virtual int GetLastPing(void) const;
	virtual int GetLowestPing(void) const;
	virtual int GetPlayerPing(const PlayerID playerId);
	virtual void StartOccasionalPing(void);
	virtual void StopOccasionalPing(void);
	virtual bool IsConnected(void) const;
	virtual unsigned int GetSynchronizedRandomInteger(void) const;
	virtual bool GenerateCompressionLayer(unsigned int inputFrequencyTable[256], bool inputLayer);
	virtual bool DeleteCompressionLayer(bool inputLayer);
	virtual void RegisterAsRemoteProcedureCall(int* uniqueID, void(*functionPointer) (RPCParameters* rpcParms));
	virtual void RegisterClassMemberRPC(int* uniqueID, void* functionPointer);
	virtual void UnregisterAsRemoteProcedureCall(int* uniqueID);
	virtual bool RPC(int* uniqueID, const char* data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp);
	virtual bool RPC(int* uniqueID, BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp);
	virtual bool RPC_(int* uniqueID, BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp, NetworkID networkID);
	virtual void SetTrackFrequencyTable(bool b);
	virtual bool GetSendFrequencyTable(unsigned int outputFrequencyTable[256]);
	virtual float GetCompressionRatio(void) const;
	virtual float GetDecompressionRatio(void) const;
	virtual void AttachPlugin(void* messageHandler);
	virtual void DetachPlugin(void* messageHandler);
	virtual BitStream* GetStaticServerData(void);
	virtual void SetStaticServerData(const char* data, const int length);
	virtual BitStream* GetStaticClientData(const PlayerID playerId);
	virtual void SetStaticClientData(const PlayerID playerId, const char* data, const int length);
	virtual void SendStaticClientDataToServer(void);
	virtual PlayerID GetServerID(void) const;
	virtual PlayerID GetPlayerID(void) const;
	virtual PlayerID GetInternalID(void) const;
	virtual const char* PlayerIDToDottedIP(const PlayerID playerId) const;
	virtual void PushBackPacket(Packet* packet, bool pushAtHead);
	virtual void SetRouterInterface(void* routerInterface);
	virtual void RemoveRouterInterface(void* routerInterface);
	virtual void SetTimeoutTime(RakNetTime timeMS);
	virtual bool SetMTUSize(int size);
	virtual int GetMTUSize(void) const;
	virtual void AllowConnectionResponseIPMigration(bool allow);
	virtual void AdvertiseSystem(const char* host, unsigned short remotePort, const char* data, int dataLength);
	virtual RakNetStatisticsStruct* const GetStatistics(void);
	virtual void ApplyNetworkSimulator(double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance);
	virtual bool IsNetworkSimulatorActive(void);
	virtual PlayerIndex GetPlayerIndex(void);
};

constexpr PlayerID UNASSIGNED_PLAYER_ID = { 0xFFFFFFFF, 0xFFFF };