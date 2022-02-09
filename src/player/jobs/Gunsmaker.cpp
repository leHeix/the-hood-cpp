#include "../../main.hpp"

static constexpr std::array<glm::vec4, 34> gunsmaker_bench_positions = {{
	{ 2553.7776, -1295.8497, 1044.1250, 1.6415 },
	{ 2556.1426, -1295.8497, 1044.1250, 1.6415 },
	{ 2558.5889, -1295.8497, 1044.1250, 1.6415 },
	{ 2560.0452, -1284.7205, 1044.1250, 90.3156 },
	{ 2560.0454, -1282.8284, 1044.1250, 87.8089 },
	{ 2558.1379, -1282.7571, 1044.1250, 272.9907 },
	{ 2558.1384, -1284.7229, 1044.1250, 272.9907 },
	{ 2552.0376, -1284.7183, 1044.1250, 89.6889 },
	{ 2552.0374, -1282.8586, 1044.1250, 89.6889 },
	{ 2550.1306, -1282.8043, 1044.1250, 269.5440 },
	{ 2550.1306, -1284.6898, 1044.1250, 269.5440 },
	{ 2544.0610, -1284.6923, 1044.1250, 90.9423 },
	{ 2544.0608, -1282.7592, 1044.1250, 90.9423 },
	{ 2542.1538, -1282.8098, 1044.1250, 272.0507 },
	{ 2542.1533, -1284.8495, 1044.1250, 272.0507 },
	{ 2541.8589, -1291.0032, 1044.1250, 178.6765 },
	{ 2544.4763, -1291.0048, 1044.1250, 178.6765 },
	{ 2544.3831, -1295.8499, 1044.1250, 0.3881 },
	{ 2542.1165, -1295.8508, 1044.1250, 0.3881 },
	{ 2542.1379, -1302.6193, 1044.1250, 270.4604 },
	{ 2542.1423, -1304.5365, 1044.1250, 269.5204 },
	{ 2544.0493, -1304.5090, 1044.1250, 91.8821 },
	{ 2544.0491, -1302.5327, 1044.1250, 91.8821 },
	{ 2550.1885, -1302.5221, 1044.1250, 267.3270 },
	{ 2550.1892, -1304.5217, 1044.1250, 269.2070 },
	{ 2552.1016, -1304.5366, 1044.1250, 93.1354 },
	{ 2552.0979, -1302.6097, 1044.1250, 92.1954 },
	{ 2558.1655, -1302.6531, 1044.1250, 268.2670 },
	{ 2558.1638, -1304.5928, 1044.1250, 268.2670 },
	{ 2560.0803, -1304.5957, 1044.1250, 95.6420 },
	{ 2560.0728, -1302.6307, 1044.1250, 86.2420 }
}};

// Mess
static std::bitset<gunsmaker_bench_positions.size()> gunsmaker_used_benchs;
static std::array<int, gunsmaker_bench_positions.size()> gunsmaker_bench_checkpoints;
static std::deque<CPlayer*> gunsmaker_queue;
static std::array<int, MAX_PLAYERS> gunsmaker_player_benchs;
static std::array<bool, MAX_PLAYERS> gunsmaker_is_in_checkpoint;

static bool GunsmakerBuildingCallback(CPlayer* player, bool enter)
{
	if (enter)
	{
		player->Notifications()->ShowBeatingText(5000, 0xED2B2B, { 100, 255 }, "Habla con el supervisor para trabajar como fabricante de armas");
	}
	else
	{
		if (player->Job() == player::job::gunsmaker)
		{
			jobs::TriggerCallback(player::job::gunsmaker, player, jobs::event::out_of_interior);
		}
	}

	return true;
}

static bool GunsmakerEvent(CPlayer* player, jobs::event event, int data)
{
	switch (event)
	{
		case jobs::event::join:
		{
			std::uint64_t sites = gunsmaker_used_benchs.to_ullong();
			int free = std::countr_one(sites);
			if (free == gunsmaker_used_benchs.size())
			{
				if (std::find(gunsmaker_queue.begin(), gunsmaker_queue.end(), player) != gunsmaker_queue.end())
				{
					player->Notifications()->ShowBeatingText(5000, 0xED2B2B, { 100, 255 }, "Ya estás en la cola");
				}
				else
				{
					gunsmaker_queue.push_back(player);
					player->Notifications()->Show("Todas las mesas están usadas. Se te notificará cuando se libere una.", 8000);
				}

				return true;
			}

			gunsmaker_player_benchs[*player] = free;
			streamer::TogglePlayerDynamicCP(*player, gunsmaker_bench_checkpoints[free], true);
			streamer::Update(*player, streamer::STREAMER_TYPE_CP);
			player->Notifications()->ShowBeatingText(7500, 0xED2B2B, { 100, 255 }, "Dirígete a tu mesa asignada para empezar a trabajar");

			break;
		}
		case jobs::event::leave:
		{
			if (player->JobData().paycheck > 0)
			{
				player->GiveMoney(player->JobData().paycheck);
				player->Notifications()->Show(fmt::format("Te pagaron ~g~{}$~w~ por tus trabajos. Vuelve a tu mesa o presiona ~y~~k~~CONVERSATION_YES~~w~ para dejar de trabajar", player->JobData().paycheck), 7000);
				player->JobData().paycheck = 0;
				return false;
			}

			int& benchid = gunsmaker_player_benchs[*player];
			if(benchid != -1)
			{
				streamer::TogglePlayerDynamicCP(*player, gunsmaker_bench_checkpoints[benchid], false);
				gunsmaker_used_benchs.set(benchid, false);
				benchid = -1;
			}

			auto queue_it = std::find(gunsmaker_queue.begin(), gunsmaker_queue.end(), player);
			if (queue_it != gunsmaker_queue.end())
			{
				gunsmaker_queue.erase(queue_it);
			}

			break;
		}
		case jobs::event::out_of_interior:
		{
			if (player->JobData().paycheck > 0)
			{
				player->Notifications()->Show(fmt::format("Fuiste despedido por salir de la fábrica. Te pagaron ~g~{}$~w~ antes de despedirte.", player->JobData().paycheck), 10000);
				player->GiveMoney(player->JobData().paycheck);
				player->JobData().paycheck = 0;
			}
			else
			{
				player->Notifications()->Show("Fuiste despedido por abandonar la fábrica.", 10000);
			}

			int& benchid = gunsmaker_player_benchs[*player];
			if (benchid != -1)
			{
				streamer::TogglePlayerDynamicCP(*player, gunsmaker_bench_checkpoints[benchid], false);
				gunsmaker_used_benchs.set(benchid, false);
				benchid = -1;
			}

			auto queue_it = std::find(gunsmaker_queue.begin(), gunsmaker_queue.end(), player);
			if (queue_it != gunsmaker_queue.end())
			{
				gunsmaker_queue.erase(queue_it);
			}

			break;
		}

		default:
			return true;
	}

	return true;
}

static public_hook _j_gm_ogmi("OnGameModeInit", [] {
	gunsmaker_player_benchs.fill(-1);

	// Actors
	streamer::CreateDynamicActor(168, 2548.1860, -1293.0232, 1045.1250, 182.7474, 0, 2);

	// Jobs
	jobs::CreatePickupSite(player::job::gunsmaker, { 2548.1860, -1293.0232, 1044.1250 }, 0, 2, -1, "{DADADA}Presiona {ED2B2B}Y {DADADA}para recibir tu paga");
	jobs::SetJobCallback(player::job::gunsmaker, GunsmakerEvent);

	// EnExs
	enter_exits->Create(19902, "{ED2B2B}Taller de armas\n{DADADA}Presiona {ED2B2B}H {DADADA}para entrar", "{DADADA}Presiona {ED2B2B}H {DADADA}para salir", { 1976.0343, -1923.4221, 13.5469, 180.1644 }, 0, 0, { 2570.4001, -1301.9230, 1044.1250, 88.4036 }, 0, 2, GunsmakerBuildingCallback);

	// MapIcons
	streamer::CreateDynamicMapIcon(1976.0343, -1923.4221, 13.5469, 18, -1);

	int tmpobjectid = streamer::CreateDynamicObject(19447, 2571.55078, -1301.67456, 1044.49414, 0.00000, 0.00000, 0.00000, 0, 2);
	streamer::SetDynamicObjectMaterial(tmpobjectid, 0, 19297, "matlights", "emergencylights64", 0x00FFFFFF);
	tmpobjectid = streamer::CreateDynamicObject(19447, 2530.55127, -1306.86475, 1048.78259, 0.00000, 0.00000, 0.00000, 0, 2);
	streamer::SetDynamicObjectMaterial(tmpobjectid, 0, 19297, "matlights", "emergencylights64", 0x00FFFFFF);

	for (size_t i = 0; auto&& site : gunsmaker_bench_positions)
	{
		gunsmaker_bench_checkpoints[i++] = streamer::CreateDynamicCP(site.x, site.y, site.z, 1.F, 0, 2);
	}

	return 1;
});

static public_hook _j_gm_opc("OnPlayerConnect", [](std::uint16_t playerid) {
	streamer::TogglePlayerAllDynamicCPs(playerid, false);
	return 1;
});

static public_hook _j_gm_opedcp("OnPlayerEnterDynamicCP", [](std::uint16_t playerid, int checkpointid) {
	auto* player = server::player_pool[playerid];
	if (player->Job() == player::job::gunsmaker && gunsmaker_player_benchs[playerid] != -1)
	{
		int benchid = gunsmaker_player_benchs[playerid];
		if (gunsmaker_bench_checkpoints[benchid] == checkpointid)
		{
			if (gunsmaker_is_in_checkpoint[playerid])
				return ~1;

			gunsmaker_is_in_checkpoint[playerid] = true;
			streamer::TogglePlayerDynamicCP(playerid, checkpointid, false);
			TogglePlayerControllable(playerid, false);

			const auto screen_blacked = [=] {
				player->ToggleWidescreen(true);
				player->Chat()->Resend();
				player->SetPosition(gunsmaker_bench_positions[benchid]);

				const auto game = [checkpointid](CPlayer* player, bool success) {
					static const std::array<const char*, 7> gun_names{{
						"un rifle rudimentario",
						"un revólver",
						"un subfusil",
						"un rifle",
						"una carabina",
						"un rifle de asalto",
						"un fusil de francotirador"
					}};

					ClearAnimations(*player, false);
					TogglePlayerControllable(*player, true);
					player->ToggleWidescreen(false);
					player->Chat()->Resend();

					size_t crafted_gun = Random::get<size_t>(0, gun_names.size() - 1);
					if (success)
					{
						player->JobData().paycheck += 150 * (crafted_gun + 1);
						player->Notifications()->Show(fmt::format("Fabricaste ~y~{}~w~. Ve con el armero para que te paguen o fabrica otra arma.", gun_names[crafted_gun]), 5000);
					}
					else
					{
						player->Notifications()->Show(fmt::format("Fallaste al construir ~r~{}~w~. Inténtalo nuevamente.", gun_names[crafted_gun]), 5000);
					}

					streamer::TogglePlayerDynamicCP(*player, checkpointid, true);
				};
				player->KeyGame()->Start(9.9F, 2.5F, game);
			};
			player->FadeScreen()->Fade(255, screen_blacked);
		}
	}
	return 1;
});

static public_hook _j_gm_opexdcp("OnPlayerExitDynamicCP", [](std::uint16_t playerid, int checkpointid) {
	auto* player = server::player_pool[playerid];
	if (player->Job() == player::job::gunsmaker && gunsmaker_player_benchs[playerid] != -1)
	{
		gunsmaker_is_in_checkpoint[playerid] = false;
		return ~1;
	}

	return 1;
});