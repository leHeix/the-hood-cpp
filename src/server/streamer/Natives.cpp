#include "../../main.hpp"

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
