#include "../main.hpp"

std::unique_ptr<CEnterExitManager> enter_exits = std::make_unique<CEnterExitManager>();

void CEnterExitManager::Create(int pickup_model, const std::string& enter_text, const std::string& exit_text, glm::vec4 enter_pos, int enter_world, int enter_interior, glm::vec4 exit_pos, int exit_world, int exit_interior, std::function<bool(CPlayer*, bool)> callback)
{
	cell info[3] = { 0x4545, static_cast<cell>(_enter_exits.size()), 1 };

	enter_exit ee;
	ee.enter.position = enter_pos;
	ee.enter.world = enter_world;
	ee.enter.interior = enter_interior;
	ee.enter.label = streamer::CreateDynamic3DTextLabel(enter_text, -1, enter_pos.x, enter_pos.y, enter_pos.z, 10.f, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, enter_world, enter_interior);
	ee.enter.pickup = streamer::CreateDynamicPickup(pickup_model, 1, enter_pos.x, enter_pos.y, enter_pos.z - 0.5f, enter_world, enter_interior);
	ee.enter.area = streamer::CreateDynamicCircle(enter_pos.x, enter_pos.y, 1.f, enter_world, enter_interior);
	streamer::data::SetArrayData(streamer::STREAMER_TYPE_AREA, ee.enter.area, streamer::E_STREAMER_EXTRA_ID, info);

	info[2] = 0;
	ee.exit.position = exit_pos;
	ee.exit.world = exit_world;
	ee.exit.interior = exit_interior;
	ee.exit.label = streamer::CreateDynamic3DTextLabel(exit_text, -1, exit_pos.x, exit_pos.y, exit_pos.z, 10.f, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, exit_world, exit_interior);
	ee.exit.pickup = streamer::CreateDynamicPickup(pickup_model, 1, exit_pos.x, exit_pos.y, exit_pos.z - 0.5f, exit_world, exit_interior);
	ee.exit.area = streamer::CreateDynamicCircle(exit_pos.x, exit_pos.y, 1.f, exit_world, exit_interior);
	streamer::data::SetArrayData(streamer::STREAMER_TYPE_AREA, ee.exit.area, streamer::E_STREAMER_EXTRA_ID, info);

	ee.callback = callback;

	_enter_exits.push_back(std::move(ee));
}

namespace ee
{
	static cell OnPlayerKeyStateChange(std::uint16_t playerid, std::uint32_t newkeys, std::uint32_t oldkeys)
	{
		if ((newkeys & KEY_CTRL_BACK) != 0)
		{
			auto areas = streamer::GetPlayerDynamicAreas(playerid);
			if (!areas.empty())
			{
				for (auto&& area : areas)
				{
					cell info[3]{};
					streamer::data::GetArrayData(streamer::STREAMER_TYPE_AREA, area, streamer::E_STREAMER_EXTRA_ID, info);
					if (info[0] == 0x4545) // 'EE'
					{
						int id = info[1];
						const auto& ee = enter_exits->Get(id);
						if (ee.callback)
						{
							if (!ee.callback(server::player_pool[playerid], info[2]))
								return ~1;
						}

						const auto& pos = (info[2] ? ee.exit : ee.enter);
						server::player_pool[playerid]->Position() = pos.position;
						server::player_pool[playerid]->VirtualWorld() = pos.world;
						server::player_pool[playerid]->Interior() = pos.interior;
						SetPlayerPos(playerid, pos.position.x, pos.position.y, pos.position.z);
						SetPlayerFacingAngle(playerid, pos.position.w);
						SetPlayerInterior(playerid, pos.interior);
						SetPlayerVirtualWorld(playerid, pos.world);

						break;
					}
				}
			}
		}

		return 1;
	}
	static CPublicHook<OnPlayerKeyStateChange> _ee_opksc("OnPlayerKeyStateChange");
}
