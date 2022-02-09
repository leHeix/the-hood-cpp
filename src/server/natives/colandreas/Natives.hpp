#pragma once

namespace colandreas
{
	constexpr auto WATER_OBJECT = 20000;

	bool Init();
	int RayCastLine(float StartX, float StartY, float StartZ, float EndX, float EndY, float EndZ, float& x, float& y, float& z);

	float FindZFor2DCoord(float x, float y, float init_z = 700.f, float end_z = -1000.f);
	bool IsAboveWater(float x, float y, float z);
}
