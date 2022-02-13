#include "../../main.hpp"

std::array<std::unique_ptr<CVehicle>, MAX_VEHICLES> vehicles::vehicle_pool;

CVehicle::CVehicle(std::uint16_t modelid, glm::vec4 position, std::pair<int, int> color)
	: _modelid(modelid), _position(std::move(position)), _color(color), _fuel(vehicles::model_data[_modelid - 400].max_fuel)
{
	_vehicleid = CreateVehicle(_modelid, _position.x, _position.y, _position.z, _position.w, _color.first, _color.second, -1, false);
	if (_vehicleid != INVALID_VEHICLE_ID)
	{
		// Fix unset parameters
		SetVehicleParamsEx(_vehicleid, 0, 0, 0, 0, 0, 0, 0);
	}
}

CVehicle::~CVehicle()
{
	StopUpdating();
	if (_timers.toggle_engine != nullptr)
	{
		_timers.toggle_engine->Killed() = true;
		timers::timer_manager->Delete(_timers.toggle_engine);
		_timers.toggle_engine = nullptr;
	}

	if (_vehicleid != INVALID_VEHICLE_ID)
	{
		DestroyVehicle(_vehicleid);
		vehicles::vehicle_pool[_vehicleid] = nullptr;
	}
}

CVehicle* CVehicle::create(std::uint16_t modelid, glm::vec4 position, std::pair<int, int> color)
{
	std::unique_ptr<CVehicle> veh{ new CVehicle(modelid, position, color) };
	if (veh->Valid())
	{
		auto id = veh->ID();
		vehicles::vehicle_pool[id] = std::move(veh);
		return vehicles::vehicle_pool[id].get();
	}

	return nullptr;
}

void CVehicle::destroy(CVehicle* vehicle)
{
	if (vehicle && vehicle->Valid())
	{
		vehicles::vehicle_pool[vehicle->ID()].reset();
	}
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

void CVehicle::ToggleEngineByPlayer(CPlayer* player)
{
	if (_timers.toggle_engine != nullptr)
	{
		player->Notifications()->ShowBeatingText(1000, 0xED2B2B, { 100, 255 }, fmt::format("El vehículo ya se está {}", (Engine() == engine_state::off ? "enciendendo" : "apagando")));
		return;
	}

	player->Notifications()->ShowBeatingText(1000, 0xF29624, { 100, 255 }, fmt::format("{} motor", (Engine() == engine_state::off ? "Encendiendo" : "Apagando")));
	_timers.toggle_engine = timers::timer_manager->Once(1000, [this,player](timers::CTimer*) {
		_timers.toggle_engine = nullptr;
		if (_health <= 375.F)
		{
			player->Notifications()->ShowBeatingText(5000, 0xED2B2B, { 100, 255 }, "Motor averiado. Llama a un mecánico");
			return;
		}

		if (_fuel <= 0.F)
		{
			player->Notifications()->ShowBeatingText(5000, 0xED2B2B, { 100, 255 }, "TANQUE SIN GASOLINA");
			return;
		}

		ToggleEngine();
		player->Notifications()->ShowBeatingText(3000, 0x98D952, { 100, 255 }, fmt::format("Motor {}", (Engine() == engine_state::off ? "apagado" : "encendido")));
	});
}

CPlayer* CVehicle::GetDriver()
{
	for (auto&& [id, player] : server::player_pool)
	{
		if (GetPlayerVehicleID(id) == _vehicleid && GetPlayerState(id) == PLAYER_STATE_DRIVER)
		{
			return player.get();
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

inline void CVehicle::SetFuel(float fuel)
{
	_fuel = std::clamp(fuel, 0.F, vehicles::model_data[_modelid - 400].max_fuel);
}

inline void CVehicle::AddFuel(float fuel)
{
	_fuel = std::clamp(_fuel + fuel, 0.F, vehicles::model_data[_modelid - 400].max_fuel);
}

inline float CVehicle::GetFuel() const
{
	return _fuel;
}

void CVehicle::SetHealth(float health)
{
	_health = health;
	SetVehicleHealth(_vehicleid, _health);
}

inline float CVehicle::GetHealth() const
{
	return _health;
}

inline void CVehicle::Repair()
{
	SetHealth(1000.F);
	int dummy, doors;
	GetVehicleDamageStatus(_vehicleid, &dummy, &doors, &dummy, &dummy);

	// Repair and reattach all doors without closing them
	doors &= ~(0b00000110'00000110'00000110'00000110);

	UpdateVehicleDamageStatus(_vehicleid, 0, doors, 0, 0);
}

void CVehicle::SetPosition(glm::vec3 position)
{
	_position.x = position.x;
	_position.y = position.y;
	_position.z = position.z;
	SetVehiclePos(_vehicleid, _position.x, _position.y, _position.z);
}

void CVehicle::SetPosition(glm::vec4 position)
{
	_position = std::move(position);
	SetVehiclePos(_vehicleid, _position.x, _position.y, _position.z);
	SetVehicleZAngle(_vehicleid, _position.w);
}

void CVehicle::SetZAngle(float angle)
{
	_position.w = angle;
	SetVehicleZAngle(_vehicleid, _position.w);
}

inline glm::vec4 CVehicle::GetPosition() const
{
	return _position;
}

void CVehicle::SetColor(std::pair<int, int> color)
{
	_color = std::move(color);
	ChangeVehicleColor(_vehicleid, _color.first, _color.second);
}

inline std::pair<int, int> CVehicle::GetColor() const
{
	return _color;
}

void CVehicle::SetPaintjob(std::uint8_t paintjob)
{
	_paintjob = paintjob;
	ChangeVehiclePaintjob(_vehicleid, _paintjob);
}

inline std::uint8_t CVehicle::GetPaintjob() const
{
	return _paintjob;
}

inline void CVehicle::SetDamageStatus(int panels, int doors, int lights, int tires)
{
	UpdateVehicleDamageStatus(_vehicleid, panels, doors, lights, tires);
}

void CVehicle::SetInterior(int id)
{
	_interior = id;
	LinkVehicleToInterior(_vehicleid, _interior);
}

inline int CVehicle::GetInterior() const
{
	return _interior;
}

void CVehicle::SetVirtualWorld(int worldid)
{
	_world = worldid;
	SetVehicleVirtualWorld(_vehicleid, _world);
}

inline int CVehicle::GetVirtualWorld() const
{
	return _world;
}

std::tuple<int, int, int, int> CVehicle::GetDamageStatus() const
{
	int panels{}, doors{}, lights{}, tires{};
	GetVehicleDamageStatus(_vehicleid, &panels, &doors, &lights, &tires);
	return std::make_tuple(panels, doors, lights, tires);
}

std::tuple<int, int, int, int, int, int, int> CVehicle::GetParams() const
{
	int engine, lights, alarm, doors, bonnet, boot, objective;
	GetVehicleParamsEx(_vehicleid, &engine, &lights, &alarm, &doors, &bonnet, &boot, &objective);
	return std::make_tuple(engine, lights, alarm, doors, bonnet, boot, objective);
}

std::bitset<7> CVehicle::GetParamsBitset() const
{
	int engine, lights, alarm, doors, bonnet, boot, objective;
	GetVehicleParamsEx(_vehicleid, &engine, &lights, &alarm, &doors, &bonnet, &boot, &objective);
	std::bitset<7> params;
	params.set(vehicles::params::engine, engine);
	params.set(vehicles::params::lights, lights);
	params.set(vehicles::params::alarm, alarm);
	params.set(vehicles::params::doors, doors);
	params.set(vehicles::params::bonnet, bonnet);
	params.set(vehicles::params::boot, boot);
	params.set(vehicles::params::objective, objective);
	return params;
}

inline std::uint16_t CVehicle::GetModel() const
{
	return _modelid;
}

void CVehicle::Lock()
{
	_locked = true;
	int engine, lights, alarm, doors, bonnet, boot, objective;
	GetVehicleParamsEx(_vehicleid, &engine, &lights, &alarm, &doors, &bonnet, &boot, &objective);
	SetVehicleParamsEx(_vehicleid, engine, lights, alarm, VEHICLE_PARAMS_ON, bonnet, boot, objective);
}

void CVehicle::Unlock()
{
	_locked = false;
	int engine, lights, alarm, doors, bonnet, boot, objective;
	GetVehicleParamsEx(_vehicleid, &engine, &lights, &alarm, &doors, &bonnet, &boot, &objective);
	SetVehicleParamsEx(_vehicleid, engine, lights, alarm, VEHICLE_PARAMS_OFF, bonnet, boot, objective);
}

inline bool CVehicle::Locked() const
{
	return _locked;
}

static public_hook _v_opsc("OnPlayerStateChange", [](std::uint16_t playerid, int newstate, int oldstate) {
	auto* player = server::player_pool[playerid];

	if (newstate == PLAYER_STATE_DRIVER)
	{
		player->Needs()->HideBars();

		if (vehicles::vehicle_pool[GetPlayerVehicleID(playerid)]->Engine() == CVehicle::engine_state::off)
		{
			player->Notifications()->ShowBeatingText(5000, 0xED2B2B, { 100, 255 }, "Presiona ~k~~CONVERSATION_NO~ para encender el vehículo");
		}
	}
	else if (oldstate == PLAYER_STATE_DRIVER)
	{
		player->Needs()->ShowBars();
	}
	
	return 1;
});
static public_hook _v_opksc("OnPlayerKeyStateChange", [](std::uint16_t playerid, int newkeys, int oldkeys) {
	if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
	{
		auto& vehicle = vehicles::vehicle_pool[GetPlayerVehicleID(playerid)];
		if ((newkeys & KEY_NO) != 0)
		{
			vehicle->ToggleEngineByPlayer(server::player_pool[playerid]);
		}
	}
	return 1;
});

static command vehcommand("vehicle", { "veh", "v" }, [](CPlayer* player, cmd::argument_store args) {
	vehicles::stModel model{};

	try
	{
		args >> model;
	}
	catch (const std::exception& e)
	{
		player->Chat()->Send(0xDADADAFF, "USO: {ED2B2B}/vehicle {DADADA}<modelo o id>");
		return;
	}

	auto* vehicle = CVehicle::create(model.id, player->Position(), { -1, -1 });
	if (!vehicle)
	{
		player->Chat()->Send(0xED2B2B, "[ERROR] {DADADA}No se pudo crear el vehículo.");
		return;
	}

	player->PutInVehicle(vehicle, 0);
	player->Chat()->Send(0xDADADAFF, "Se creó un {{ED2B2B}}{}{{DADADA}} en tu posición.", model.name);
});

static command repairvehcommand("repairveh", { "rv" }, [](CPlayer* player, cmd::argument_store args) {
	try
	{
		CVehicle* vehicle = nullptr;
		args >> vehicle;
		vehicle->Repair();
		
		player->Chat()->Send(0xED2B2BFF, "Vehículo ID {{ED2B2B}}{}{{DADADA}} reparado.", vehicle->ID());
	}
	catch (const std::exception& e)
	{
		if (!IsPlayerInAnyVehicle(*player))
		{
			player->Chat()->Send(0xDADADAFF, "USO: {ED2B2B}/repairveh {DADADA}[id]");
			return;
		}

		vehicles::vehicle_pool[GetPlayerVehicleID(*player)]->Repair();
		player->Chat()->Send(0xED2B2BFF, "Vehículo ID {{ED2B2B}}{}{{DADADA}} reparado.", GetPlayerVehicleID(*player));
	}
});