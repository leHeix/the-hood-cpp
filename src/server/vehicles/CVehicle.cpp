#include "../../main.hpp"

CVehicle::CVehicle(std::uint16_t modelid, glm::vec4 position, std::pair<int, int> color)
	: _modelid(modelid), _position(std::move(position)), _color(color), _fuel(vehicles::model_data[_modelid - 400].max_fuel)
{
	_vehicleid = CreateVehicle(_modelid, _position.x, _position.y, _position.z, _position.w, _color.first, _color.second, -1, false);
	if (_vehicleid != INVALID_VEHICLE_ID)
	{
		// Fix unset parameters
		SetVehicleParamsEx(_vehicleid, 0, 0, 0, 0, 0, 0, 0);
	}

	vehicles::vehicle_pool[_vehicleid] = this;
}

CVehicle::~CVehicle()
{
	StopUpdating();
	if (_vehicleid != INVALID_VEHICLE_ID)
	{
		DestroyVehicle(_vehicleid);
		vehicles::vehicle_pool[_vehicleid] = nullptr;
	}
}

CVehicle* CVehicle::create(std::uint16_t modelid, glm::vec4 position, std::pair<int, int> color)
{
	auto veh = std::make_unique<CVehicle>(modelid, position, color);
	if (veh->Valid())
	{
		auto id = veh->ID();
		vehicles::vehicle_pool[id] = std::move(veh);
		return vehicles::vehicle_pool[id].get();
	}

	return nullptr;
}

void CVehicle::StartUpdating()
{
	_timers.update = timers::timer_manager->Repeat(1000, 1000, std::bind(&CVehicle::Update, this, std::placeholders::_1));
}

void CVehicle::StopUpdating()
{
	if (_timers.update != nullptr)
	{
		_timers.update->Killed() = true;
		timers::timer_manager->Delete(_timers.update);
		_timers.update = nullptr;
	}
}

void CVehicle::ToggleEngine(CVehicle::engine_state state)
{
	if (Engine() == state)
		return;

	int engine, lights, alarm, doors, bonnet, boot, objective;
	GetVehicleParamsEx(_vehicleid, &engine, &lights, &alarm, &doors, &bonnet, &boot, &objective);
	SetVehicleParamsEx(_vehicleid, (state == engine_state::default_state ? !engine : static_cast<int>(state)), lights, alarm, doors, bonnet, boot, objective);

	if (Engine() == engine_state::on)
	{
		StartUpdating();
	}
	else if (_timers.update)
	{
		StopUpdating();
	}
}

CPlayer* CVehicle::GetDriver()
{
	for (auto&& [id, player] : server::player_pool)
	{
		if (GetPlayerVehicleID(id) == _vehicleid && GetPlayerState(id) == PLAYER_STATE_DRIVER)
		{
			return player;
		}
	}

	return nullptr;
}

void CVehicle::Update(timers::CTimer* timer)
{
	if (Engine() == engine_state::on)
	{
		if (_health <= 375.F)
		{
			auto* driver = GetDriver();
			if (driver != nullptr)
			{
				driver->Notifications()->ShowBeatingText(5000, 0xED2B2B, { 100, 255 }, "Motor averiado. Llama a un mecánico.");
			}

			ToggleEngine(engine_state::off);
		}

		_fuel -= (GetSpeed() + 0.1) / VEHICLE_FUEL_DIVISOR;
		if (_fuel <= 0.0)
		{
			auto* driver = GetDriver();
			if (driver != nullptr)
			{
				driver->Notifications()->ShowBeatingText(10000, 0xED2B2B, { 100, 255 }, "TANQUE SIN GASOLINA");
			}

			ToggleEngine(engine_state::off);
		}
	}
}

bool CVehicle::RegisterToPlayer(CPlayer* player)
{
	if (_owner)
		return false;

	_owner = player;
	player->Vehicles().push_back(this);

	int panels, doors, lights, tires;
	GetVehicleDamageStatus(_vehicleid, &panels, &doors, &lights, &tires);

	int params{ 0 }, engine, lights_p, alarm, doors_p, bonnet, boot, objective;
	GetVehicleParamsEx(_vehicleid, &engine, &lights_p, &alarm, &doors_p, &bonnet, &boot, &objective);
	params |= engine;
	params |= (lights_p << 1);
	params |= (alarm << 2);
	params |= (doors_p << 3);
	params |= (bonnet << 4);
	params |= (boot << 5);
	params |= (objective << 6);

	std::string components_str;
	for (auto&& component : _components)
	{
		components_str += fmt::format("{},", component);
	}

	components_str.pop_back();

	try
	{
		auto stmt = server::database->PrepareLock(
		   "INSERT INTO `PLAYER_VEHICLES` "
				"(OWNER_ID, MODEL, HEALTH, FUEL, PANELS_STATUS, DOORS_STATUS, LIGHTS_STATUS, TIRES_STATUS, COLOR_ONE, COLOR_TWO, PAINTJOB, POS_X, POS_Y, POS_Z, ANGLE, INTERIOR, VW, COMPONENTS, PARAMS) "
		   "VALUES "
				"(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
		);

		stmt->Bind<1>(player->AccountId()); // OWNER_ID
		stmt->Bind<2>(_modelid); // MODEL
		stmt->Bind<3>(_health); // HEALTH
		stmt->Bind<4>(_fuel); // FUEL
		stmt->Bind<5>(panels); // PANELS_STATUS
		stmt->Bind<6>(doors); // DOORS_STATUS
		stmt->Bind<7>(lights); // LIGHTS_STATUS
		stmt->Bind<8>(tires); // TIRES_STATUS
		stmt->Bind<9>(_color.first); // COLOR_ONE
		stmt->Bind<10>(_color.second); // COLOR_TWO
		stmt->Bind<11>(_paintjob); // PAINTJOB
		stmt->Bind<12>(_position.x); // POS_X
		stmt->Bind<13>(_position.y); // POS_Y
		stmt->Bind<14>(_position.z); // POS_Z
		stmt->Bind<15>(_position.w); // ANGLE
		stmt->Bind<16>(_interior); // INTERIOR
		stmt->Bind<17>(_world); // VW
		stmt->Bind<18>(components_str); // COMPONENTS
		stmt->Bind<19>(params); // PARAMS

		stmt->Step();

		_dbid = stmt->LastInsertId();
	}
	catch (const std::exception& e)
	{
		sampgdk::logprintf("[player:vehicles!] Failed to register vehicle to player %s (ID %i):", player->Name().c_str(), player->PlayerId());
		sampgdk::logprintf("[player:vehicles!]    %s", e.what());
		return false;
	}

	return true;
}