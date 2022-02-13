#include "../../main.hpp"

struct stLawnmowerArea
{
	int area;
	CPlayer* using_player{ nullptr };

	struct stGrass
	{
		int object;
		int area;
	};
	std::vector<stGrass> grass;
	int initial_grass_count;

	struct
	{
		std::pair<glm::vec3, glm::vec3> area;
		glm::vec4 spawn;
	} positions;
};

static std::array<stLawnmowerArea, 3> _lawnmower_areas;
static std::array<int, MAX_PLAYERS> _player_park{ -1 };

static void GenerateGrassInSquare(int park_id, glm::vec3 pos1, glm::vec3 pos2)
{
	for (int i = 0, j = Random::get(50, 100); i < j; ++i)
	{
		bool is_above_water{ false };
		glm::vec3 grass_pos;

		do
		{
			grass_pos = glm::linearRand(pos1, pos2);
			is_above_water = (colandreas::RayCastLine(grass_pos.x, grass_pos.y, 100.f, grass_pos.x, grass_pos.y, -100.f, grass_pos.x, grass_pos.y, grass_pos.z) == colandreas::WATER_OBJECT);
			if (grass_pos.z > 35.f)
				is_above_water = true;
		} while (is_above_water);

		grass_pos.z = colandreas::FindZFor2DCoord(grass_pos.x, grass_pos.y, 100.f, -100.f);
		auto grass_obj = streamer::CreateDynamicObject(817, grass_pos.x, grass_pos.y, grass_pos.z + 0.6, 0.0, 0.0, 0.0, 0, 0);
		auto area = streamer::CreateDynamicCircle(grass_pos.x, grass_pos.y, 1.2, 0, 0, _lawnmower_areas[park_id].using_player->PlayerId());
		cell info[2] = { 'MOW', grass_obj };
		streamer::data::SetArrayData(streamer::STREAMER_TYPE_AREA, area, streamer::E_STREAMER_EXTRA_ID, info);

		_lawnmower_areas[park_id].grass.push_back({ grass_obj, area });
	}

	_lawnmower_areas[park_id].initial_grass_count = _lawnmower_areas[park_id].grass.size();
}

static bool LawnmowerEvent(CPlayer* player, jobs::event event, int area)
{
	switch (event)
	{
		case jobs::event::join:
		{
			if (_lawnmower_areas[area].using_player != nullptr)
			{
				player->Notifications()->Show("Ya hay alguien cortando este parque. Dirígete a otro o espera.", 8000);
				return false;
			}

			TogglePlayerControllable(*player, false);
			
			player->FadeScreen()->Fade(255, [player, area] {
				CVehicle* lawnmower = CVehicle::create(572, { 2052.7703, -1242.6202, 23.6974, 85.6861 }, { -1, -1 });
				lawnmower->ToggleEngine(CVehicle::engine_state::on);
				player->PutInVehicle(lawnmower, 0);

				player->SetData<CVehicle*>("lawnmower:vehicle", lawnmower);

				_player_park[player->PlayerId()] = area;
				_lawnmower_areas[area].using_player = player;
				GenerateGrassInSquare(area, _lawnmower_areas[area].positions.area.first, _lawnmower_areas[area].positions.area.second);

				player->FadeScreen()->Fade(100, true, [player, area] {
					player->Notifications()->ShowBeatingText(15000, 0xED2B2B, { 75, 255 }, fmt::format("Empieza a cortar el césped. Te quedan {} matorrales", _lawnmower_areas[area].grass.size()));
					TogglePlayerControllable(*player, true);
					PlayAudioStreamForPlayer(*player, "https://cdn.discordapp.com/attachments/883089457329344523/938212731952181268/lawnmower.mp3", 0.0, 0.0, 0.0, 0.0, false);
					player->Chat()->Clear();
				});
			});

			break;
		}
		case jobs::event::out_of_vehicle:
		{
			TogglePlayerControllable(*player, false);

			player->FadeScreen()->Fade(255, [player]() {
				StopAudioStreamForPlayer(*player);

				auto* lawnmower = player->GetData<CVehicle*>("lawnmower:vehicle").value_or(nullptr);
				player->RemoveData("lawnmower:vehicle");
				CVehicle::destroy(lawnmower);

				player->SetPosition(_lawnmower_areas[_player_park[*player]].positions.spawn);

				auto& area = _lawnmower_areas[_player_park[player->PlayerId()]];
				for (auto&& grass : area.grass)
				{
					streamer::DestroyDynamicObject(grass.object);
					streamer::DestroyDynamicArea(grass.area);
				}
				area.grass.clear();

				player->FadeScreen()->Fade(100, false, [player]() {
					TogglePlayerControllable(*player, true);
					player->Notifications()->Show("~y~El capataz~w~ te vio fuera del cortacesped y ~r~te despidió~w~.", 10000);

					_lawnmower_areas[_player_park[player->PlayerId()]].using_player = nullptr;
					_player_park[player->PlayerId()] = -1;
					});
				});

			break;
		}
	}
	return true;
}

public_hook j_lm_opd("OnPlayerDisconnect", +[](std::uint16_t playerid, std::uint8_t reason) -> cell {
	auto* player = server::player_pool[playerid];
	if (player->HasData("lawnmower:vehicle"))
	{
		CVehicle::destroy(player->GetData<CVehicle*>("lawnmower:vehicle").value_or(nullptr));
	}

	if (_player_park[playerid] != -1)
	{
		auto& area = _lawnmower_areas[_player_park[playerid]];
		for (auto&& grass : area.grass)
		{
			streamer::DestroyDynamicObject(grass.object);
			streamer::DestroyDynamicArea(grass.area);
		}
		area.grass.clear();
		area.using_player = nullptr;
		_player_park[playerid] = -1;
	}

	return 1;
});

public_hook _j_lm_ogmi("OnGameModeInit", +[]() -> cell {
	sampgdk::logprintf("[jobs:lawnmower] Initializing...");

	int actor = streamer::CreateDynamicActor(16, 2081.3334, -1241.6908, 23.9750, 93.9778, true, 100.f, 0, 0);
	streamer::ApplyDynamicActorAnimation(actor, "SMOKING", "null", 4.1, false, 0, 0, 0, 0);
	streamer::ApplyDynamicActorAnimation(actor, "SMOKING", "M_SMKLEAN_LOOP", 4.1, true, 0, 0, 1, 0);

	_lawnmower_areas[0].area = streamer::CreateDynamicRectangle(2055.0747, -1248.8661, 1981.7301, -1148.3273, 0, 0);
	streamer::data::SetIntData(streamer::STREAMER_TYPE_AREA, _lawnmower_areas[0].area, streamer::E_STREAMER_EXTRA_ID, 'A_LM');
	_lawnmower_areas[0].positions.area = { glm::vec3{ 2055.0747, -1248.8661, 23.8589 }, glm::vec3{ 1981.7301, -1148.3273, 21.2429 } };
	_lawnmower_areas[0].positions.spawn = { 2052.7703, -1242.6202 ,23.6974, 85.6861 };

	jobs::CreatePickupSite(player::job::lawnmower, { 2081.5234, -1241.6908, 23.9750 }, 0, 0, 0);
	jobs::SetJobCallback(player::job::lawnmower, LawnmowerEvent);

	return 1;
});

static public_hook j_lm_opeedm("OnPlayerExitDynamicArea", +[](std::uint16_t playerid, int areaid) {
	int data = streamer::data::GetIntData(streamer::STREAMER_TYPE_AREA, areaid, streamer::E_STREAMER_EXTRA_ID);
	if (data == 'A_LM')
	{
		auto* player = server::player_pool[playerid];
		if (player->Job() == player::job::lawnmower)
		{
			player->Job() = player::job::none;
			TogglePlayerControllable(playerid, false);

			player->FadeScreen()->Fade(255, [player]() {
				if (player->HasData("lawnmower:vehicle"))
				{
					CVehicle::destroy(player->GetData<CVehicle*>("lawnmower:vehicle").value_or(nullptr));
					player->RemoveData("lawnmower:vehicle");
				}

				auto park_id = _player_park[player->PlayerId()];
				player->SetPosition(_lawnmower_areas[park_id].positions.spawn);

				auto& area = _lawnmower_areas[_player_park[player->PlayerId()]];
				for (auto&& grass : area.grass)
				{
					streamer::DestroyDynamicObject(grass.object);
					streamer::DestroyDynamicArea(grass.area);
				}
				area.grass.clear();
				area.using_player = nullptr;
				_player_park[player->PlayerId()] = -1;

				TogglePlayerControllable(*player, true);
				player->Notifications()->Show("~y~El capataz~w~ te vio fuera del parque y ~r~te despidió~w~.", 15000);
			});
		}
	}

	return 1;
});

static public_hook _j_lm_opsc("OnPlayerStateChange", +[](std::uint16_t playerid, int newstate, int oldstate) {
	auto* player = server::player_pool[playerid];
	if (player->Job() == player::job::lawnmower)
	{
		if (newstate == PLAYER_STATE_ONFOOT && oldstate == PLAYER_STATE_DRIVER)
		{
			jobs::TriggerCallback(player::job::lawnmower, player, jobs::event::out_of_vehicle, _player_park[playerid]);
		}
	}

	return 1;
});

static public_hook _j_lm_opeda("OnPlayerEnterDynamicArea", +[](std::uint16_t playerid, int areaid) -> cell {
	cell info[2]{};
	streamer::data::GetArrayData(streamer::STREAMER_TYPE_AREA, areaid, streamer::E_STREAMER_EXTRA_ID, info);
	if (info[0] == 'MOW')
	{
		PlayerPlaySound(playerid, 20800, 0.f, 0.f, 0.f);

		streamer::DestroyDynamicObject(info[1]);
		streamer::DestroyDynamicArea(areaid);

		auto& area = _lawnmower_areas[_player_park[playerid]];
		area.grass.erase(std::remove_if(area.grass.begin(), area.grass.end(), [areaid](const auto& v) { return v.area == areaid; }));
		
		server::player_pool[playerid]->Chat()->Send(0xDADADAFF, "Te quedan {{ED2B2B}}{}{{DADADA}} matorrales.", area.grass.size());

		if (area.grass.empty())
		{
			SetVehicleParamsEx(GetPlayerVehicleID(playerid), 0, 0, 0, 0, 0, 0, 0);
			TogglePlayerControllable(playerid, false);

			server::player_pool[playerid]->FadeScreen()->Fade(255, [playerid]() {
				auto* player = server::player_pool[playerid];

				StopAudioStreamForPlayer(playerid);
				player->Job() = player::job::none;
				CVehicle::destroy(player->GetCurrentVehicle());
				player->SetPosition({ 2052.7703, -1242.6202, 23.6974 });

				server::player_pool[playerid]->FadeScreen()->Fade(100, false, [playerid]() {
					TogglePlayerControllable(playerid, true);
					auto paycheck = _lawnmower_areas[_player_park[playerid]].initial_grass_count * 5;
					server::player_pool[playerid]->GiveMoney(paycheck);
					server::player_pool[playerid]->Notifications()->Show(fmt::format("Terminaste tu trabajo como cortacésped y te pagaron ~g~{}$~w~ por tu labor.", paycheck), 10000);
					
					_lawnmower_areas[_player_park[playerid]].using_player = nullptr;
					_player_park[playerid] = -1;
				});
			});
		}

		return ~1;
	}

	return 1;
});
