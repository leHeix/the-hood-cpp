#pragma once

namespace streamer
{
	enum types 
	{
		STREAMER_TYPE_OBJECT,
		STREAMER_TYPE_PICKUP,
		STREAMER_TYPE_CP,
		STREAMER_TYPE_RACE_CP,
		STREAMER_TYPE_MAP_ICON,
		STREAMER_TYPE_3D_TEXT_LABEL,
		STREAMER_TYPE_AREA,
		STREAMER_TYPE_ACTOR,

		STREAMER_MAX_TYPES
	};

	enum area_types
	{
		STREAMER_AREA_TYPE_CIRCLE,
		STREAMER_AREA_TYPE_CYLINDER,
		STREAMER_AREA_TYPE_SPHERE,
		STREAMER_AREA_TYPE_RECTANGLE,
		STREAMER_AREA_TYPE_CUBOID,
		STREAMER_AREA_TYPE_POLYGON,

		STREAMER_MAX_AREA_TYPES
	};

	enum object_types
	{
		STREAMER_OBJECT_TYPE_GLOBAL,
		STREAMER_OBJECT_TYPE_PLAYER,
		STREAMER_OBJECT_TYPE_DYNAMIC,

		STREAMER_MAX_OBJECT_TYPES
	};

	enum
	{
		E_STREAMER_AREA_ID,
		E_STREAMER_ATTACHED_OBJECT,
		E_STREAMER_ATTACHED_PLAYER,
		E_STREAMER_ATTACHED_VEHICLE,
		E_STREAMER_ATTACH_OFFSET_X,
		E_STREAMER_ATTACH_OFFSET_Y,
		E_STREAMER_ATTACH_OFFSET_Z,
		E_STREAMER_ATTACH_R_X,
		E_STREAMER_ATTACH_R_Y,
		E_STREAMER_ATTACH_R_Z,
		E_STREAMER_ATTACH_X,
		E_STREAMER_ATTACH_Y,
		E_STREAMER_ATTACH_Z,
		E_STREAMER_COLOR,
		E_STREAMER_DRAW_DISTANCE,
		E_STREAMER_EXTRA_ID,
		E_STREAMER_HEALTH,
		E_STREAMER_INTERIOR_ID,
		E_STREAMER_INVULNERABLE,
		E_STREAMER_MAX_X,
		E_STREAMER_MAX_Y,
		E_STREAMER_MAX_Z,
		E_STREAMER_MIN_X,
		E_STREAMER_MIN_Y,
		E_STREAMER_MIN_Z,
		E_STREAMER_MODEL_ID,
		E_STREAMER_MOVE_R_X,
		E_STREAMER_MOVE_R_Y,
		E_STREAMER_MOVE_R_Z,
		E_STREAMER_MOVE_SPEED,
		E_STREAMER_MOVE_X,
		E_STREAMER_MOVE_Y,
		E_STREAMER_MOVE_Z,
		E_STREAMER_NEXT_X,
		E_STREAMER_NEXT_Y,
		E_STREAMER_NEXT_Z,
		E_STREAMER_PLAYER_ID,
		E_STREAMER_PRIORITY,
		E_STREAMER_ROTATION,
		E_STREAMER_R_X,
		E_STREAMER_R_Y,
		E_STREAMER_R_Z,
		E_STREAMER_SIZE,
		E_STREAMER_STREAM_DISTANCE,
		E_STREAMER_STYLE,
		E_STREAMER_SYNC_ROTATION,
		E_STREAMER_TEST_LOS,
		E_STREAMER_TYPE,
		E_STREAMER_WORLD_ID,
		E_STREAMER_X,
		E_STREAMER_Y,
		E_STREAMER_Z
	};

	constexpr int INVALID_STREAMER_ID = 0;
	constexpr float STREAMER_OBJECT_SD = 300.0;
	constexpr float STREAMER_OBJECT_DD = 0.0;
	constexpr float STREAMER_PICKUP_SD = 200.0;
	constexpr float STREAMER_CP_SD = 200.0;
	constexpr float STREAMER_RACE_CP_SD = 200.0;
	constexpr float STREAMER_MAP_ICON_SD = 200.0;
	constexpr float STREAMER_3D_TEXT_LABEL_SD = 200.0;
	constexpr float STREAMER_ACTOR_SD = 200.0;

	// Areas
	cell CreateDynamicCircle(float x, float y, float size, int worldid = -1, int interiorid = -1, int playerid = -1, int priority = 0);
	cell CreateDynamicCylinder(float x, float y, float minz, float maxz, float size, int worldid = -1, int interiorid = -1, int playerid = -1);
	cell CreateDynamicSphere(float x, float y, float z, float size, int worldid = -1, int interiorid = -1, int playerid = -1);
	cell GetPlayerNumberDynamicAreas(int playerid);
	std::vector<int> GetPlayerDynamicAreas(int playerid);

	// 3D Text Labels
	cell CreateDynamic3DTextLabel(const std::string& text, int color, float x, float y, float z, float drawdistance, int attachedplayer = INVALID_PLAYER_ID, int attachedvehicle = INVALID_VEHICLE_ID, bool testlos = 0, int worldid = -1, int interiorid = -1, int playerid = -1, float streamdistance = STREAMER_3D_TEXT_LABEL_SD, int areaid = -1, int priority = 0);
	cell DestroyDynamic3DTextLabel(cell id);
	bool IsValidDynamic3DTextLabel(cell id);
	std::string GetDynamic3DTextLabelText(cell id);
	cell UpdateDynamic3DTextLabelText(cell id, int color, const std::string& text);

	// Pickups
	cell CreateDynamicPickup(int modelid, int type, float x, float y, float z, int worldid = -1, int interiorid = -1, int playerid = -1, float streamdistance = STREAMER_PICKUP_SD, int areaid = -1, int priority = 0);
	cell DestroyDynamicPickup(cell pickupid);
	bool IsValidDynamicPickup(cell pickupid);

	// Actors
	int CreateDynamicActor(int modelid, float x, float y, float z, float r, bool invulnerable = true, float health = 100.0, int worldid = -1, int interiorid = -1, int playerid = -1, float streamdistance = STREAMER_ACTOR_SD, int areaid = -1, int priority = 0);
	int DestroyDynamicActor(int actorid);

	// Map Icons
	int CreateDynamicMapIcon(float x, float y, float z, int type, uint32_t color, int worldid = -1, int interiorid = -1, int playerid = -1, float streamdistance = STREAMER_MAP_ICON_SD, int style = MAPICON_LOCAL, int areaid = -1, int priority = 0);
	int DestroyDynamicMapIcon(int iconid);
	bool IsValidDynamicMapIcon(int iconid);

	// Data Manipulation
	namespace data
	{
		bool GetFloatData(int type, int id, int data, float& result);
		bool SetFloatData(int type, int id, int data, float value);
		int GetIntData(int type, int id, int data);
		bool SetIntData(int type, int id, int data, int value);

		template<std::size_t N>
		bool GetArrayData(int type, int id, int data, cell(&dest)[N])
		{
			static AMX_NATIVE native = sampgdk::FindNative("Streamer_GetArrayData");
			return sampgdk::InvokeNative(native, fmt::format(FMT_COMPILE("iiiA[{}]i"), N).c_str(), type, id, data, &dest[0], N);
		}

		template<std::size_t N>
		bool GetArrayData(int type, int id, int data, std::array<cell, N>& dest)
		{
			static AMX_NATIVE native = sampgdk::FindNative("Streamer_GetArrayData");
			return sampgdk::InvokeNative(native, fmt::format(FMT_COMPILE("iiiA[{}]i"), N).c_str(), type, id, data, dest.data(), N);
		}

		template<std::size_t N>
		bool SetArrayData(int type, int id, int data, const cell(&dest)[N])
		{
			static AMX_NATIVE native = sampgdk::FindNative("Streamer_SetArrayData");
			return sampgdk::InvokeNative(native, fmt::format(FMT_COMPILE("iiia[{}]i"), N).c_str(), type, id, data, &dest[0], N);
		}

		template<std::size_t N>
		bool SetArrayData(int type, int id, int data, const std::array<cell, N>& dest)
		{
			static AMX_NATIVE native = sampgdk::FindNative("Streamer_GetArrayData");
			return sampgdk::InvokeNative(native, fmt::format(FMT_COMPILE("iiiA[{}]i"), N).c_str(), type, id, data, dest.data(), N);
		}

		bool IsInArrayData(int type, int id, int data, int value);
		bool AppendArrayData(int type, int id, int data, int value);
		bool RemoveArrayData(int type, int id, int data, int value);
		int GetArrayDataLength(int type, int id, int data);
		int GetUpperBound(int type);
	}
}