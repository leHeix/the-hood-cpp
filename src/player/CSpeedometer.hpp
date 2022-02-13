#pragma once

class CSpeedometer
{
	CPlayer* _player;
	CVehicle* _target_vehicle{ nullptr };
	timers::CTimer* _update_timer{ nullptr };

	void Update();
public:
	explicit CSpeedometer(CPlayer* player);
	~CSpeedometer();

	void Show(CVehicle* vehicle);
	void Hide();
};