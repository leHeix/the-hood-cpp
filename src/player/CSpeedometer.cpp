#include "../main.hpp"

CSpeedometer::CSpeedometer(CPlayer* player)
	: _player(player)
{
	textdraw_manager.LoadFile("speedometer.toml", "speedometer");
}

CSpeedometer::~CSpeedometer()
{
	if (_update_timer)
	{
		_update_timer->Killed() = true;
		timers::timer_manager->Delete(_update_timer);
		_update_timer = nullptr;
	}
}

void CSpeedometer::Update()
{
	if (!_target_vehicle || !_target_vehicle->Valid())
		return;

	if (_player->Paused())
		return;

	auto* textdraws = textdraw_manager["speedometer"];

	constexpr auto MAX_SLASHES = 33;
	constexpr auto MAX_FLOORS = 35;

	int speed_kmh = _target_vehicle->GetSpeed();
	textdraws->GetPlayerTextDraws(_player)[2]->SetText(std::to_string(speed_kmh));

	const std::uint16_t max_speed = vehicles::GetModelData(_target_vehicle->GetModel()).max_speed;
	int max_speed_percentage = (static_cast<float>(speed_kmh) / static_cast<float>(max_speed)) * 100.F;
	int slashes = std::clamp(((max_speed_percentage * MAX_SLASHES) / 100), 0, MAX_SLASHES);

	std::string td_string;
	td_string.reserve(MAX_SLASHES + MAX_FLOORS + 3); // About the magic number: it's the length of the newline
	td_string.insert(td_string.begin(), slashes, '/');
	td_string += "~n~";

	int gas_percentage = (_target_vehicle->GetFuel() / vehicles::GetModelData(_target_vehicle->GetModel()).max_fuel) * 100.F;
	int floors = std::clamp(((gas_percentage * MAX_FLOORS) / 100), 0, MAX_FLOORS);

	td_string.insert(td_string.end(), floors, '-');
	textdraws->GetPlayerTextDraws(_player)[0]->SetText(std::move(td_string));
}

void CSpeedometer::Show(CVehicle* vehicle)
{
	_target_vehicle = vehicle;
	Update();
	textdraw_manager["speedometer"]->Show(_player);
	if (!_update_timer)
	{
		_update_timer = timers::timer_manager->Repeat(1000, 1000, [this](timers::CTimer* timer) {
			Update();
		});
	}
}

void CSpeedometer::Hide()
{
	if (_update_timer)
	{
		_update_timer->Killed() = true;
		timers::timer_manager->Delete(_update_timer);
		_update_timer = nullptr;
	}

	_target_vehicle = nullptr;
	textdraw_manager["speedometer"]->Hide(_player);
}