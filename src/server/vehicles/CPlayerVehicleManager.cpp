#include "../../main.hpp"

CPlayerVehicleManager::CPlayerVehicleManager(CPlayer* player)
	: _player(player), _speedometer(std::make_unique<CSpeedometer>(_player))
{
}

void CPlayerVehicleManager::Load()
{
	std::thread([this] {
		auto stmt = server::database->Prepare("SELECT * FROM `PLAYER_VEHICLES` WHERE `OWNER_ID` = ?;");
		stmt->Bind<1>(_player->AccountId()); // OWNER_ID

		do
		{
			stmt->Step();

			if (!stmt->HasRow())
				break;

			auto row = stmt->Row();
			uint16_t modelid = *row->Get<int>("MODEL");
			std::pair<int, int> color = { *row->Get<int>("COLOR_ONE"), *row->Get<int>("COLOR_TWO") };
			glm::vec4 position = { *row->Get<float>("POS_X"), *row->Get<float>("POS_Y"), *row->Get<float>("POS_Z"), *row->Get<float>("ANGLE") };

			auto* vehicle = CVehicle::create(modelid, position, color);
			vehicle->DbId() = *row->Get<int>("VEHICLE_ID");
		} while (!stmt->Finished());

	}).detach();
}

bool CPlayerVehicleManager::Register(CVehicle* vehicle)
{
	_vehicles.push_back(vehicle);
	vehicle->Owner() = _player;
	
	std::string components_str;
	for (auto&& component : vehicle->Components())
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

		stmt->Bind<1>(_player->AccountId()); // OWNER_ID
		stmt->Bind<2>(vehicle->GetModel()); // MODEL
		stmt->Bind<3>(vehicle->GetHealth()); // HEALTH
		stmt->Bind<4>(vehicle->GetFuel()); // FUEL
		auto [panels, doors, lights, tires] = vehicle->GetDamageStatus();
		stmt->Bind<5>(panels); // PANELS_STATUS
		stmt->Bind<6>(doors); // DOORS_STATUS
		stmt->Bind<7>(lights); // LIGHTS_STATUS
		stmt->Bind<8>(tires); // TIRES_STATUS
		auto color = vehicle->GetColor();
		stmt->Bind<9>(color.first); // COLOR_ONE
		stmt->Bind<10>(color.second); // COLOR_TWO
		stmt->Bind<11>(vehicle->GetPaintjob()); // PAINTJOB
		glm::vec4 position = vehicle->GetPosition();
		stmt->Bind<12>(position.x); // POS_X
		stmt->Bind<13>(position.y); // POS_Y
		stmt->Bind<14>(position.z); // POS_Z
		stmt->Bind<15>(position.w); // ANGLE
		stmt->Bind<16>(vehicle->GetInterior()); // INTERIOR
		stmt->Bind<17>(vehicle->GetVirtualWorld()); // VW
		stmt->Bind<18>(components_str); // COMPONENTS
		stmt->Bind<19>(vehicle->GetParamsBitset().to_ulong()); // PARAMS

		stmt->Step();

		vehicle->DbId() = stmt->LastInsertId();

		return true;
	}
	catch (const std::exception& e)
	{
		sampgdk::logprintf("[player:vehicles!] Failed to register vehicle to player %s (ID %i):", _player->Name().c_str(), _player->PlayerId());
		sampgdk::logprintf("[player:vehicles!]    %s", e.what());
	}

	return false;
}

static command rvtpcommand("registervehicle", { "rvp" }, [](CPlayer* player, cmd::argument_store args) {
	CVehicle* vehicle;
	CPlayer* destination;

	try
	{
		args >> vehicle;
		args >> destination;
	}
	catch (const std::exception& e)
	{
		player->Chat()->Send(0xDADADAFF, "USO: {ED2B2B}/registervehicle {DADADA}<vehículo> <jugador>");
		return;
	}

	if (!destination->Vehicles()->Register(vehicle))
	{
		player->Chat()->Send(0xED2B2BFF, "[ERROR] {DADADA}No se puedo registrar el vehículo en la base de datos.");
		return;
	}
	
	player->Chat()->Send(0xDADADAFF, "Se añadió un {{ED2B2B}}{}{{DADADA}} (ID {{ED2B2B}}{}{{DADADA}}) a la cuenta de {{ED2B2B}}{}{{DADADA}}.", vehicles::names[vehicle->GetModel() - 400], vehicle->ID(), destination->Name());
	destination->Chat()->Send(0xDADADAFF, "El administrador {{ED2B2B}}{}{{DADADA}} agregó un {{ED2B2B}}{}{{DADADA}} a tu cuenta.", player->Name(), vehicles::names[vehicle->GetModel() - 400]);
});