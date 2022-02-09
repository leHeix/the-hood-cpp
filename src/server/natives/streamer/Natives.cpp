#include "../../../main.hpp"

// - Areas

cell streamer::CreateDynamicCircle(float x, float y, float size, int worldid, int interiorid, int playerid, int priority)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicCircle");
	return sampgdk::InvokeNative(native, "fffiiii", x, y, size, worldid, interiorid, playerid, priority);
}

cell streamer::CreateDynamicCylinder(float x, float y, float minz, float maxz, float size, int worldid, int interiorid, int playerid)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicCylinder");
	return sampgdk::InvokeNative(native, "fffffiii", x, y, minz, maxz, size, worldid, interiorid, playerid);
}

cell streamer::CreateDynamicSphere(float x, float y, float z, float size, int worldid, int interiorid, int playerid )
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicSphere");
	return sampgdk::InvokeNative(native, "ffffiii", x, y, z, size, worldid, interiorid, playerid);
}

int streamer::CreateDynamicRectangle(float minx, float miny, float maxx, float maxy, int worldid, int interiorid, int playerid, int priority)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicRectangle");
	return sampgdk::InvokeNative(native, "ffffiiii", minx, miny, maxx, maxy, worldid ,interiorid, playerid, priority);
}

int streamer::DestroyDynamicArea(int areaid)
{
	static AMX_NATIVE native = sampgdk::FindNative("DestroyDynamicArea");
	return sampgdk::InvokeNative(native, "i", areaid);
}

cell streamer::GetPlayerNumberDynamicAreas(int playerid)
{
	static AMX_NATIVE native = sampgdk::FindNative("GetPlayerNumberDynamicAreas");
	return sampgdk::InvokeNative(native, "i", playerid);
}

std::vector<int> streamer::GetPlayerDynamicAreas(int playerid)
{
	std::vector<int> areas;

	if (int num = GetPlayerNumberDynamicAreas(playerid); num > 0)
	{
		areas.resize(num);
		
		static AMX_NATIVE native = sampgdk::FindNative("GetPlayerDynamicAreas");
		sampgdk::InvokeNative(native, fmt::format("iA[{}]i", areas.size()).c_str(), playerid, areas.data(), areas.size());
	}

	return areas;
}

// - TextLabels

cell streamer::CreateDynamic3DTextLabel(const std::string& text, int color, float x, float y, float z, float drawdistance, int attachedplayer, int attachedvehicle, bool testlos, int worldid, int interiorid, int playerid, float streamdistance, int areaid, int priority)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamic3DTextLabel");
	return sampgdk::InvokeNative(native, "siffffiibiiifii", text.c_str(), color, x, y, z, drawdistance, attachedplayer, attachedvehicle, testlos, worldid, interiorid, playerid, streamdistance, areaid, priority);
}

cell streamer::DestroyDynamic3DTextLabel(cell id)
{
	static AMX_NATIVE native = sampgdk::FindNative("DestroyDynamic3DTextLabel");
	return sampgdk::InvokeNative(native, "i", id);
}

bool streamer::IsValidDynamic3DTextLabel(cell id)
{
	static AMX_NATIVE native = sampgdk::FindNative("IsValidDynamic3DTextLabel");
	return sampgdk::InvokeNative(native, "i", id);
}

std::string streamer::GetDynamic3DTextLabelText(cell id)
{
	static AMX_NATIVE native = sampgdk::FindNative("GetDynamic3DTextLabelText");
	std::string text(1024, '\0');
	sampgdk::InvokeNative(native, fmt::format("iS[{}]i", text.max_size()).c_str(), id, text.data(), text.max_size());
	text.shrink_to_fit();
	return text;
}

cell streamer::UpdateDynamic3DTextLabelText(cell id, int color, const std::string& text)
{
	static AMX_NATIVE native = sampgdk::FindNative("UpdateDynamic3DTextLabelText");
	return sampgdk::InvokeNative(native, "iis", id, color, text.c_str());
}

// - Pickups

cell streamer::CreateDynamicPickup(int modelid, int type, float x, float y, float z, int worldid, int interiorid, int playerid, float streamdistance, int areaid, int priority)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicPickup");
	return sampgdk::InvokeNative(native, "iifffiiifii", modelid, type, x, y, z, worldid, interiorid, playerid, streamdistance, areaid, priority);
}

cell streamer::DestroyDynamicPickup(cell pickupid)
{
	static AMX_NATIVE native = sampgdk::FindNative("DestroyDynamicPickup");
	return sampgdk::InvokeNative(native, "i", pickupid);
}

bool streamer::IsValidDynamicPickup(cell pickupid)
{
	static AMX_NATIVE native = sampgdk::FindNative("IsValidDynamicPickup");
	return sampgdk::InvokeNative(native, "i", pickupid);
}

// - Actors

int streamer::CreateDynamicActor(int modelid, float x, float y, float z, float r, bool invulnerable, float health, int worldid, int interiorid, int playerid, float streamdistance, int areaid, int priority)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicActor");
	return sampgdk::InvokeNative(native, "iffffbfiiifii", modelid, x, y, z, r, invulnerable, health, worldid, interiorid, playerid, streamdistance, areaid, priority);
}

int streamer::DestroyDynamicActor(int actorid)
{
	static AMX_NATIVE native = sampgdk::FindNative("DestroyDynamicActor");
	return sampgdk::InvokeNative(native, "i", actorid);
}

int streamer::ApplyDynamicActorAnimation(int actorid, const std::string_view animlib, const std::string_view animname, float fdelta, bool loop, int lockx, int locky, int freeze, int time)
{
	static AMX_NATIVE native = sampgdk::FindNative("ApplyDynamicActorAnimation");
	return sampgdk::InvokeNative(native, "issfbiiii", actorid, animlib.data(), animname.data(), fdelta, loop, lockx, locky, freeze, time);
}

// - Map Icons

int streamer::CreateDynamicMapIcon(float x, float y, float z, int type, uint32_t color, int worldid, int interiorid, int playerid, float streamdistance, int style, int areaid, int priority)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicMapIcon");
	return sampgdk::InvokeNative(native, "fffiiiiifiii", x, y, z, type, color, worldid, interiorid, playerid, streamdistance, style, areaid, priority);
}

int streamer::DestroyDynamicMapIcon(int iconid)
{
	static AMX_NATIVE native = sampgdk::FindNative("DestroyDynamicMapIcon");
	return sampgdk::InvokeNative(native, "i", iconid);
}

bool streamer::IsValidDynamicMapIcon(int iconid)
{
	static AMX_NATIVE native = sampgdk::FindNative("IsValidDynamicMapIcon");
	return !!sampgdk::InvokeNative(native, "i", iconid);
}

// - Objects

int streamer::CreateDynamicObject(int modelid, float x, float y, float z, float rx, float ry, float rz, int worldid, int interiorid, int playerid, float streamdistance, float drawdistance, int areaid, int priority)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicObject");
	return sampgdk::InvokeNative(native, "iffffffiiiffii", modelid, x, y, z, rx, ry, rz, worldid, interiorid, playerid, streamdistance, drawdistance, areaid, priority);
}

int streamer::DestroyDynamicObject(int objectid)
{
	static AMX_NATIVE native = sampgdk::FindNative("DestroyDynamicObject");
	return sampgdk::InvokeNative(native, "i", objectid);
}

int streamer::SetDynamicObjectMaterial(int objectid, int materialindex, int modelid, const std::string& txdname, const std::string& texturename, int materialcolor)
{
	static AMX_NATIVE native = sampgdk::FindNative("SetDynamicObjectMaterial");
	return sampgdk::InvokeNative(native, "iiissi", objectid, materialindex, modelid, txdname.c_str(), texturename.c_str(), materialcolor);
}

// - Checkpoints

int streamer::CreateDynamicCP(float x, float y, float z, float size, int worldid, int interiorid, int playerid, float streamdistance, int areaid, int priority)
{
	static AMX_NATIVE native = sampgdk::FindNative("CreateDynamicCP");
	return sampgdk::InvokeNative(native, "ffffiiifii", x, y, z, size, worldid, interiorid, playerid, streamdistance, areaid, priority);
}

int streamer::DestroyDynamicCP(int checkpointid)
{
	static AMX_NATIVE native = sampgdk::FindNative("DestroyDynamicCP");
	return sampgdk::InvokeNative(native, "i", checkpointid);
}

bool streamer::IsValidDynamicCP(int checkpointid)
{
	static AMX_NATIVE native = sampgdk::FindNative("IsValidDynamicCP");
	return sampgdk::InvokeNative(native, "i", checkpointid);
}

int streamer::TogglePlayerDynamicCP(int playerid, int checkpointid, bool toggle)
{
	static AMX_NATIVE native = sampgdk::FindNative("TogglePlayerDynamicCP");
	return sampgdk::InvokeNative(native, "iib", playerid, checkpointid, toggle);
}

int streamer::TogglePlayerAllDynamicCPs(int playerid, bool toggle)
{
	static AMX_NATIVE native = sampgdk::FindNative("TogglePlayerAllDynamicCPs");
	const cell exceptions[1] = { -1 };
	return sampgdk::InvokeNative(native, "iba[1]i", playerid, toggle, exceptions, 1);
}

bool streamer::IsPlayerInDynamicCP(int playerid, int checkpointid)
{
	static AMX_NATIVE native = sampgdk::FindNative("IsPlayerInDynamicCP");
	return sampgdk::InvokeNative(native, "ii", playerid, checkpointid);
}

int streamer::GetPlayerVisibleDynamicCP(int playerid)
{
	static AMX_NATIVE native = sampgdk::FindNative("GetPlayerVisibleDynamicCP");
	return sampgdk::InvokeNative(native, "i", playerid);
}

// - Updates

void streamer::ProcessActiveItems()
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_ProcessActiveItems");
	sampgdk::InvokeNative(native, "");
}

bool streamer::Update(int playerid, streamer::types type)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_Update");
	return sampgdk::InvokeNative(native, "ii", playerid, static_cast<int>(type));
}

bool streamer::UpdateEx(int playerid, float x, float y, float z, int worldid, int interiorid, int type, int compensatedtime, bool freezeplayer)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_UpdateEx");
	return sampgdk::InvokeNative(native, "ifffiiiib", playerid, x, y, z, worldid, interiorid, type, compensatedtime, freezeplayer);
}


// - Data

bool streamer::data::GetFloatData(int type, int id, int data, float& result)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_GetFloatData");
	return sampgdk::InvokeNative(native, "iiiR", type, id, data, reinterpret_cast<cell*>(&result));
}

bool streamer::data::SetFloatData(int type, int id, int data, float value)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_SetFloatData");
	return sampgdk::InvokeNative(native, "iiif", type, id, data, value);
}

int streamer::data::GetIntData(int type, int id, int data)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_GetIntData");
	return sampgdk::InvokeNative(native, "iii", type, id, data);
}

bool streamer::data::SetIntData(int type, int id, int data, int value)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_SetIntData");
	return sampgdk::InvokeNative(native, "iiii", type, id, data, value);
}

bool streamer::data::IsInArrayData(int type, int id, int data, int value)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_IsInArrayData");
	return sampgdk::InvokeNative(native, "iiii", type, id, data, value);
}

bool streamer::data::AppendArrayData(int type, int id, int data, int value)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_AppendArrayData");
	return sampgdk::InvokeNative(native, "iiii", type, id, data, value);
}

bool streamer::data::RemoveArrayData(int type, int id, int data, int value)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_RemoveArrayData");
	return sampgdk::InvokeNative(native, "iiii", type, id, data, value);
}

int streamer::data::GetArrayDataLength(int type, int id, int data)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_GetArrayDataLength");
	return sampgdk::InvokeNative(native, "iii", type, id, data);
}

int streamer::data::GetUpperBound(int type)
{
	static AMX_NATIVE native = sampgdk::FindNative("Streamer_GetUpperBound");
	return sampgdk::InvokeNative(native, "i", type);
}
