#include "../../main.hpp"

void jobs::CreatePickupSite(player::job jobid, glm::vec3 position, int vw, int interior, int extra_data, const std::string& extra_text)
{
	std::string labelstring = fmt::format("Trabajo de {{ED2B2B}}{}{{DADADA}}\nPresione {{ED2B2B}}Y{{DADADA}} para empezar a trabajar\n{}", jobs::job_names[static_cast<size_t>(jobid)], extra_text);
	
	streamer::CreateDynamic3DTextLabel(labelstring, 0xDADADAFF, position.x, position.y, position.z, 10.f, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, true, vw, interior);
	int area = streamer::CreateDynamicCircle(position.x, position.y, 1.f, vw, interior);
	cell info[3] = { 'JOB', static_cast<cell>(jobid), extra_data };
	streamer::data::SetArrayData(streamer::STREAMER_TYPE_AREA, area, streamer::E_STREAMER_EXTRA_ID, info);
}

static std::unordered_map<player::job, jobs::callback_t> _job_callbacks;

void jobs::SetJobCallback(player::job jobid, jobs::callback_t cb)
{
	_job_callbacks[jobid] = std::move(cb);
}

bool jobs::TriggerCallback(player::job jobid, CPlayer* player, jobs::event event, int data)
{
	if (_job_callbacks.contains(jobid))
	{
		return _job_callbacks[jobid](player, event, data);
	}

	return false;
}

public_hook _j_opksc("OnPlayerKeyStateChange", +[](std::uint16_t playerid, std::uint32_t newkeys, std::uint32_t oldkeys) {
	if ((newkeys & KEY_YES) != 0)
	{
		auto areas = streamer::GetPlayerDynamicAreas(playerid);
		auto* player = server::player_pool[playerid];

		for (auto&& area : areas)
		{
			cell info[3];
			streamer::data::GetArrayData(streamer::STREAMER_TYPE_AREA, area, streamer::E_STREAMER_EXTRA_ID, info);
			if (info[0] == 'JOB')
			{
				player::job job = static_cast<player::job>(info[1]);
				auto& cb = _job_callbacks[job];

				if (player->Job() == player::job::none)
				{
					if (cb)
					{
						if (!cb(player, jobs::event::join, info[2]))
						{
							return ~1;
						}
					}

					player->Job() = job;
				}
				else if (player->Job() == job)
				{
					if (cb)
					{
						if (!cb(player, jobs::event::leave, info[2]))
						{
							return ~1;
						}
					}

					player->Job() = player::job::none;
				}

				return ~1;
			}
		}
	}

	return 1;
});
