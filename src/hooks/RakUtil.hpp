#pragma once

#define MAX_OBJECT_MATERIALS 16

namespace net
{
	/*
	enum eObjectMaterialSize : unsigned char
	{
		OBJECT_MATERIAL_SIZE_32x32 = 10,
		OBJECT_MATERIAL_SIZE_64x32 = 20,
		OBJECT_MATERIAL_SIZE_64x64 = 30,
		OBJECT_MATERIAL_SIZE_128x32 = 40,
		OBJECT_MATERIAL_SIZE_128x64 = 50,
		OBJECT_MATERIAL_SIZE_128x128 = 60,
		OBJECT_MATERIAL_SIZE_256x32 = 70,
		OBJECT_MATERIAL_SIZE_256x64 = 80,
		OBJECT_MATERIAL_SIZE_256x128 = 90,
		OBJECT_MATERIAL_SIZE_256x256 = 100,
		OBJECT_MATERIAL_SIZE_512x64 = 110,
		OBJECT_MATERIAL_SIZE_512x128 = 120,
		OBJECT_MATERIAL_SIZE_512x256 = 130,
		OBJECT_MATERIAL_SIZE_512x512 = 140
	};
	*/

	enum eObjectMaterialType
	{
		OBJECT_NO_MATERIAL,
		OBJECT_MATERIAL,
		OBJECT_MATERIAL_TEXT
	};

	struct stObjectMaterial
	{
		unsigned short material_id;
		unsigned short model_id;

		unsigned int material_color = 0;

		std::string txd_name;
		std::string texture_name;

		stObjectMaterial() = default;
		stObjectMaterial(unsigned short materialid, unsigned short modelid, const std::string& txdname, const std::string& texturename, unsigned int color)
			: material_id(materialid), model_id(modelid), txd_name(txdname), texture_name(texturename), material_color(color) { };
	};

	struct stObjectMaterialText
	{
		bool bold = true;

		unsigned char text_alignment = 0;
		unsigned char font_size = 24;

		unsigned short material_id = 0;

		unsigned int font_color = 0xFFFFFFFF;
		unsigned int back_color = 0;

		unsigned char material_size = OBJECT_MATERIAL_SIZE_256x128;

		std::string text;
		std::string font_face = "Arial";

		stObjectMaterialText() = default;
		stObjectMaterialText(const std::string& text_, unsigned short materialid, unsigned char materialsize, const std::string& fontface,
			unsigned char fontsize, bool bold_, unsigned int fontcolor, unsigned int backgroundcolor, unsigned char textalign)
			: text(text_), material_id(materialid), material_size(materialsize), font_face(fontface), font_size(fontsize), bold(bold_),
			font_color(fontcolor), back_color(backgroundcolor), text_alignment(textalign) { }
	};

	struct stObjectMaterialEntry
	{
		eObjectMaterialType type = OBJECT_NO_MATERIAL;
		void* material;
	};

	namespace raknet
	{
		enum PacketEnumeration
		{
			ID_INTERNAL_PING = 6,
			ID_PING,
			ID_PING_OPEN_CONNECTIONS,
			ID_CONNECTED_PONG,
			ID_REQUEST_STATIC_DATA,
			ID_CONNECTION_REQUEST,
			ID_AUTH_KEY,
			ID_BROADCAST_PINGS = 14,
			ID_SECURED_CONNECTION_RESPONSE,
			ID_SECURED_CONNECTION_CONFIRMATION,
			ID_RPC_MAPPING,
			ID_SET_RANDOM_NUMBER_SEED = 19,
			ID_RPC,
			ID_RPC_REPLY,
			ID_DETECT_LOST_CONNECTIONS = 23,
			ID_OPEN_CONNECTION_REQUEST,
			ID_OPEN_CONNECTION_REPLY,
			ID_OPEN_CONNECTION_COOKIE,
			ID_RSA_PUBLIC_KEY_MISMATCH = 28,
			ID_CONNECTION_ATTEMPT_FAILED,
			ID_NEW_INCOMING_CONNECTION = 30,
			ID_NO_FREE_INCOMING_CONNECTIONS = 31,
			ID_DISCONNECTION_NOTIFICATION,
			ID_CONNECTION_LOST,
			ID_CONNECTION_REQUEST_ACCEPTED,
			ID_CONNECTION_BANNED = 36,
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
			ID_ADVERTISE_SYSTEM = 55,

			ID_PLAYER_SYNC = 207,
			ID_MARKERS_SYNC = 208,
			ID_UNOCCUPIED_SYNC = 209,
			ID_TRAILER_SYNC = 210,
			ID_PASSENGER_SYNC = 211,
			ID_SPECTATOR_SYNC = 212,
			ID_AIM_SYNC = 203,
			ID_VEHICLE_SYNC = 200,
			ID_RCON_COMMAND = 201,
			ID_RCON_RESPONCE = 202,
			ID_WEAPONS_UPDATE = 204,
			ID_STATS_UPDATE = 205,
			ID_BULLET_SYNC = 206,
			ID_VOICE_PACKET = 251,
		};

		enum RPCEnumeration
		{
			RPC_ClickPlayer = 23,
			RPC_ClientJoin = 25,
			RPC_EnterVehicle = 26,
			RPC_SelectObject = 27,
			RPC_ScriptCash = 31,
			RPC_ServerCommand = 50,
			RPC_Spawn = 52,
			RPC_Death = 53,
			RPC_NPCJoin = 54,
			RPC_DialogResponse = 62,
			RPC_ClickTextDraw = 83,
			RPC_SCMEvent = 96,
			RPC_PickedUpWeapon = 97,
			RPC_Chat = 101,
			RPC_SrvNetStats = 102,
			RPC_ClientCheck = 103,
			RPC_DamageVehicle = 106,
			RPC_GiveTakeDamage = 115,
			RPC_EditAttachedObject = 116,
			RPC_EditObject = 117,
			RPC_SetInteriorId = 118,
			RPC_MapMarker = 119,
			RPC_RequestClass = 128,
			RPC_RequestSpawn = 129,
			RPC_ConnectionRejected = 130,
			RPC_PickedUpPickup = 131,
			RPC_MenuSelect = 132,
			RPC_VehicleDestroyed = 136,
			RPC_MenuQuit = 140,
			RPC_ExitVehicle = 154,
			RPC_UpdateScoresPingsIPs = 155,
			RPC_CameraTarget = 168,
			RPC_ObjectNoCameraCol = 169,
			RPC_GiveDamageActor = 177,

			RPC_SetPlayerName = 11,
			RPC_SetPlayerPos = 12,
			RPC_SetPlayerPosFindZ = 13,
			RPC_SetPlayerHealth = 14,
			RPC_TogglePlayerControllable = 15,
			RPC_PlaySound = 16,
			RPC_SetPlayerWorldBounds = 17,
			RPC_GivePlayerMoney = 18,
			RPC_SetPlayerFacingAngle = 19,
			RPC_ResetPlayerMoney = 20,
			RPC_ResetPlayerWeapons = 21,
			RPC_GivePlayerWeapon = 22,
			RPC_SetVehicleParamsEx = 24,
			RPC_CancelEdit = 28,
			RPC_SetPlayerTime = 29,
			RPC_ToggleClock = 30,
			RPC_WorldPlayerAdd = 32,
			RPC_SetPlayerShopName = 33,
			RPC_SetPlayerSkillLevel = 34,
			RPC_SetPlayerDrunkLevel = 35,
			RPC_Create3DTextLabel = 36,
			RPC_DisableCheckpoint = 37,
			RPC_SetRaceCheckpoint = 38,
			RPC_DisableRaceCheckpoint = 39,
			RPC_GameModeRestart = 40,
			RPC_PlayAudioStream = 41,
			RPC_StopAudioStream = 42,
			RPC_RemoveBuildingForPlayer = 43,
			RPC_CreateObject = 44,
			RPC_SetObjectPos = 45,
			RPC_SetObjectRot = 46,
			RPC_DestroyObject = 47,
			RPC_DeathMessage = 55,
			RPC_SetPlayerMapIcon = 56,
			RPC_RemoveVehicleComponent = 57,
			RPC_Remove3DTextLabel = 58,
			RPC_ChatBubble = 59,
			RPC_UpdateSystemTime = 60,
			RPC_ShowDialog = 61,
			RPC_DestroyPickup = 63,
			RPC_WeaponPickupDestroy = 64,
			RPC_LinkVehicleToInterior = 65,
			RPC_SetPlayerArmour = 66,
			RPC_SetPlayerArmedWeapon = 67,
			RPC_SetSpawnInfo = 68,
			RPC_SetPlayerTeam = 69,
			RPC_PutPlayerInVehicle = 70,
			RPC_RemovePlayerFromVehicle = 71,
			RPC_SetPlayerColor = 72,
			RPC_DisplayGameText = 73,
			RPC_ForceClassSelection = 74,
			RPC_AttachObjectToPlayer = 75,
			RPC_InitMenu = 76,
			RPC_ShowMenu = 77,
			RPC_HideMenu = 78,
			RPC_CreateExplosion = 79,
			RPC_ShowPlayerNameTagForPlayer = 80,
			RPC_AttachCameraToObject = 81,
			RPC_InterpolateCamera = 82,
			RPC_SelectTextDraw = 83,
			RPC_SetObjectMaterial = 84,
			RPC_GangZoneStopFlash = 85,
			RPC_ApplyAnimation = 86,
			RPC_ClearAnimations = 87,
			RPC_SetPlayerSpecialAction = 88,
			RPC_SetPlayerFightingStyle = 89,
			RPC_SetPlayerVelocity = 90,
			RPC_SetVehicleVelocity = 91,
			RPC_SetPlayerDrunkVisuals = 92,
			RPC_ClientMessage = 93,
			RPC_SetWorldTime = 94,
			RPC_CreatePickup = 95,
			RPC_SetVehicleTireStatus = 98,
			RPC_MoveObject = 99,
			RPC_EnableStuntBonusForPlayer = 104,
			RPC_TextDrawSetString = 105,
			RPC_SetCheckpoint = 107,
			RPC_GangZoneCreate = 108,
			RPC_Widescreen = 111,
			RPC_PlayCrimeReport = 112,
			RPC_SetPlayerAttachedObject = 113,
			RPC_GangZoneDestroy = 120,
			RPC_GangZoneFlash = 121,
			RPC_StopObject = 122,
			RPC_SetNumberPlate = 123,
			RPC_TogglePlayerSpectating = 124,
			RPC_PlayerSpectatePlayer = 126,
			RPC_PlayerSpectateVehicle = 127,
			RPC_SetPlayerWantedLevel = 133,
			RPC_ShowTextDraw = 134,
			RPC_TextDrawHideForPlayer = 135,
			RPC_ServerJoin = 137,
			RPC_ServerQuit = 138,
			RPC_InitGame = 139,
			RPC_RemovePlayerMapIcon = 144,
			RPC_SetPlayerAmmo = 145,
			RPC_SetPlayerGravity = 146,
			RPC_SetVehicleHealth = 147,
			RPC_AttachTrailerToVehicle = 148,
			RPC_DetachTrailerFromVehicle = 149,
			RPC_SetPlayerDrunkHandling = 150,
			RPC_PickUpWeaponPickup = 151,
			RPC_SetWeather = 152,
			RPC_SetPlayerSkin = 153,
			RPC_SetPlayerInterior = 156,
			RPC_SetPlayerCameraPos = 157,
			RPC_SetPlayerCameraLookAt = 158,
			RPC_SetVehiclePos = 159,
			RPC_SetVehicleZAngle = 160,
			RPC_SetVehicleParamsForPlayer = 161,
			RPC_SetCameraBehindPlayer = 162,
			RPC_WorldPlayerRemove = 163,
			RPC_WorldVehicleAdd = 164,
			RPC_WorldVehicleRemove = 165,
			RPC_WorldPlayerDeath = 166,
			RPC_DisableVehicleCollision = 167,
			RPC_EnableCameraTargeting = 170,
			RPC_WorldActorAdd = 171,
			RPC_WorldActorRemove = 172,
			RPC_ApplyActorAnimation = 173,
			RPC_ClearActorAnimation = 174,
			RPC_SetActorFacingAngle = 175,
			RPC_SetActorPos = 176,
			RPC_SetActorHealth = 178
		};
	};

#pragma pack(push, 1)
	struct stVector
	{
		float X, Y, Z;
	};

	struct st2DVector
	{
		float fX, fY;
	};

	struct stQuat
	{
		float X, Y, Z, W;
	};

	struct stTextDrawTransmit
	{
		union
		{
			unsigned char byteFlags;
			struct
			{
				unsigned char byteBox : 1;
				unsigned char byteLeft : 1;
				unsigned char byteRight : 1;
				unsigned char byteCenter : 1;
				unsigned char byteProportional : 1;
				unsigned char bytePadding : 3;
			};
		};

		float fLetterWidth = 0.6f;
		float fLetterHeight = 2.0f;
		unsigned long dwLetterColor = 0xFFE1E1E1; // ABGR
		float fLineWidth = 400.0f;
		float fLineHeight = 633.0f;
		unsigned long dwBoxColor = 0x50000000; // ABGR
		unsigned char byteShadow = 0;
		unsigned char byteOutline = 1;
		unsigned long dwBackgroundColor = 0xFF000000; // ABGR
		unsigned char byteStyle = 1;
		unsigned char byteSelectable = 0;
		float fX = 320.0f;
		float fY = 213.0f;
		uint16_t iModelIndex = 0;
		stVector rgfRotation{ 0.0, 0.0, 0.0 };
		float fZoom = 1.0;
		uint16_t iColor1 = -1, iColor2 = -1;

		stTextDrawTransmit()
		{
			byteCenter = true;
			byteProportional = true;
		}
	};

	struct stSpawnInfo
	{
		unsigned char bTeam;
		int iSkin;
		unsigned char bUnknown;
		float fPosition[3];
		float fRotation;
		int iSpawnWeapons[3];
		int iSpawnWeaponsAmmo[3];
	};

	struct stNewVehicle
	{
		unsigned short VehicleId;
		int iVehicleType;
		float vecPos[3];
		float fRotation;
		unsigned char aColor1;
		unsigned char aColor2;
		float fHealth;
		unsigned char byteInterior;
		unsigned int dwDoorDamageStatus;
		unsigned int dwPanelDamageStatus;
		unsigned char byteLightDamageStatus;
		unsigned char byteTireDamageStatus;
		unsigned char byteAddSiren;
		unsigned char byteModSlots[14];
		unsigned char bytePaintjob;
		unsigned int cColor1;
		unsigned int cColor2;
		//BYTE	  byteUnk;
	};

	struct stOnFootSyncData
	{
		unsigned short lrAnalog;
		unsigned short udAnalog;
		unsigned short wKeys;
		stVector vecPos;
		float fQuaternion[4];
		unsigned char byteHealth;
		unsigned char byteArmour;
		unsigned char byteCurrentWeapon;
		unsigned char byteSpecialAction;
		stVector vecMoveSpeed;
		stVector vecSurfOffsets;
		unsigned short wSurfInfo;
		int iCurrentAnimationID;
	};

	struct stAimSyncData
	{
		unsigned char byteCamMode;
		float vecAimf1[3];
		float vecAimPos[3];
		float fAimZ;
		unsigned char byteWeaponState : 2;	// see eWeaponState
		unsigned char byteCamExtZoom : 6;	// 0-63 normalized
		unsigned char bAspectRatio;
	};

	struct stBulletSyncData
	{
		unsigned char bHitType;
		unsigned short iHitID;
		float fHitOrigin[3];
		float fHitTarget[3];
		float fCenterOfHit[3];
		unsigned char bWeaponID;
	};

	struct stVehicleSyncData
	{
		unsigned short VehicleID;
		unsigned short lrAnalog;
		unsigned short udAnalog;
		unsigned short wKeys;
		float fQuaternion[4];
		float vecPos[3];
		float vecMoveSpeed[3];
		float fCarHealth;
		unsigned char bytePlayerHealth;
		unsigned char bytePlayerArmour;
		unsigned char byteCurrentWeapon;
		bool byteSirenOn;
		bool byteLandingGearState;
		//unsigned short TrailerID_or_ThrustAngle;
		unsigned short TrailerID;
		//float fTrainSpeed;
		union {
			unsigned short HydraThrustAngle[2];
			float          fTrainSpeed;
		};
	};

	struct stUnoccupiedVehicleSyncData
	{
		unsigned short VehicleID;
		short VecRoll[3];
		short VecDirection[3];
		unsigned char unk[13];
		float vecPos[3];
		float vecMoveSpeed[3];
		float vecTurnSpeed[3];
		float fHealth;
	};

	struct stPassengerSyncData
	{
		unsigned short VehicleID;
		unsigned char byteSeatFlags : 7;
		unsigned char byteDriveBy : 1;
		unsigned char byteCurrentWeapon;
		unsigned char bytePlayerHealth;
		unsigned char bytePlayerArmour;
		unsigned short lrAnalog;
		unsigned short udAnalog;
		unsigned short wKeys;
		float VecPos[3];
	};

	struct stTrailerSyncData
	{
		unsigned short TrailerID;
		float fPosition[3];
		float fQuaternion[4];
		float fSpeed[3];
		float fTurnSpeed[3];
	};
#pragma pack(pop)
}