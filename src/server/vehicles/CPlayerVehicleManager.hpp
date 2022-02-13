#pragma once

class CSpeedometer;

class CPlayerVehicleManager
{
	CPlayer* _player;
	std::unique_ptr<CSpeedometer> _speedometer;
	std::vector<CVehicle*> _vehicles;
	bool _vehicles_loaded{ false };

public:
	explicit CPlayerVehicleManager(CPlayer* player);

	void Load();
	bool Register(CVehicle* vehicle);

	[[nodiscard]] auto* Speedometer() noexcept { return _speedometer.get(); }
	[[nodiscard]] const auto* Speedometer() const noexcept { return _speedometer.get(); }
	inline decltype(_vehicles)::iterator begin() noexcept(noexcept(_vehicles.begin())) { return _vehicles.begin(); }
	inline decltype(_vehicles)::iterator end() noexcept(noexcept(_vehicles.end())) { return _vehicles.end(); }
	inline decltype(_vehicles)::const_iterator begin() const noexcept(noexcept(_vehicles.begin())) { return _vehicles.begin(); }
	inline decltype(_vehicles)::const_iterator end() const noexcept(noexcept(_vehicles.end())) { return _vehicles.end(); }
};