#include "../../../main.hpp"

bool colandreas::Init()
{
	static AMX_NATIVE native = sampgdk::FindNative("CA_Init");
	return sampgdk::InvokeNative(native, "");
}

int colandreas::RayCastLine(float StartX, float StartY, float StartZ, float EndX, float EndY, float EndZ, float& x, float& y, float& z)
{
	static AMX_NATIVE native = sampgdk::FindNative("CA_RayCastLine");
	return sampgdk::InvokeNative(native, "ffffffRRR", StartX, StartY, StartZ, EndX, EndY, EndZ, &x, &y, &z);
}

float colandreas::FindZFor2DCoord(float x, float y, float init_z, float end_z)
{
	float z;
	colandreas::RayCastLine(x, y, init_z, x, y, end_z, x, y, z);
	return z;
}

bool colandreas::IsAboveWater(float x, float y, float z)
{
	return colandreas::RayCastLine(x, y, z, x, y, -1000.f, x, y, z) == WATER_OBJECT;
}
