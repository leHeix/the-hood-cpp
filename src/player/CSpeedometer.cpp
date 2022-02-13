#include "../main.hpp"

CSpeedometer::CSpeedometer(CPlayer* player)
	: _player(player)
{
	textdraw_manager.LoadFile("speedometer.toml", "speedometer");
}

void CSpeedometer::Update()
{
	constexpr auto MAX_SLASHES = 33;
	constexpr auto MAX_FLOORS = 35;

	int speed_kmh = _target_vehicle->GetSpeed();

}

void CSpeedometer::Show(CVehicle* vehicle)
{
	_target_vehicle = vehicle;
	Update();
	textdraw_manager["speedometer"]->Show(_player);
}