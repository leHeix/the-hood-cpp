#pragma once

namespace jobs
{
	constexpr const std::string_view job_names[] = {
		"Ninguno",
		"Cortacesped",
		"Fabricante de armas"
	};

	enum class event : std::uint8_t 
	{
		join = 0,
		leave,
		out_of_vehicle,
		out_of_interior
	};

	using callback_t = std::function<bool(CPlayer*, jobs::event, int)>;

	void CreatePickupSite(player::job jobid, glm::vec3 position, int vw, int interior, int extra_data = -1, const std::string& extra_text = "");
	void SetJobCallback(player::job jobid, callback_t cb);
	bool TriggerCallback(player::job jobid, CPlayer* player, jobs::event event, int data = -1);
}
