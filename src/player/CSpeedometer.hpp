#pragma once

class CSpeedometer
{
	CPlayer* _player;
	CVehicle* _target_vehicle{ nullptr };

	void Update();
public:
	explicit CSpeedometer(CPlayer* player);

	void Show(CVehicle* vehicle);
	void Hide();
};