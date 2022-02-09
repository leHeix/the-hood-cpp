#include "../main.hpp"

std::unique_ptr<std::unordered_multimap<std::string, callbacks::hook>> callbacks::_hooks{ nullptr };
std::unique_ptr<std::unordered_multimap<std::string, callbacks::hook>> callbacks::_prehooks{ nullptr };
std::unique_ptr<std::unordered_multimap<std::string, callbacks::hook>> callbacks::_posthooks{ nullptr };

PLUGIN_EXPORT bool PLUGIN_CALL OnPublicCall(AMX* amx, const char* name, cell* params, cell* retval)
{
	if (!callbacks::_hooks)
	{
		return true;
	}

	// fixme: make an appropiate way to do this
	if (!strcmp("OnPlayerClickTextDraw", name) && params[2] == INVALID_TEXT_DRAW)
	{
		auto playerid = static_cast<std::uint16_t>(params[1]);
		std::optional<std::chrono::steady_clock::time_point> tick = std::nullopt;
		tick.swap(server::player_pool[playerid]->_cancel_td_tick);

		if (!tick || std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - *tick) > std::chrono::milliseconds{ 50 + GetPlayerPing(playerid) })
		{
			name = "OnPlayerCancelTextDrawSelection";
			params[0] = sizeof(cell);
		}
		else
		{
			return true;
		}
	}

	std::string name_str{ name };

	// Process prehooks
	if (callbacks::_prehooks)
	{
		auto ph_range = callbacks::_prehooks->equal_range(name_str);
		for (auto it = ph_range.first; it != ph_range.second; ++it)
		{
			cell ret = it->second.call(amx, params);
			if (ret == ~0 || ret == ~1)
			{
				if (retval)
					*retval = ~ret;
				break;
			}
			else
			{
				if (retval)
					*retval = ret;
			}
		}
	}

	// Process hooks
	auto range = callbacks::_hooks->equal_range(name_str);
	for (auto it = range.first; it != range.second; ++it)
	{
		cell ret = it->second.call(amx, params);
		if (ret == ~0 || ret == ~1)
		{
			if(retval)
				*retval = ~ret;
			break;
		}
		else
		{
			if(retval)
				*retval = ret;
		}
	}

	// Process posthooks
	if (callbacks::_posthooks)
	{
		auto posth_range = callbacks::_posthooks->equal_range(name_str);
		for (auto it = posth_range.first; it != posth_range.second; ++it)
		{
			cell ret = it->second.call(amx, params);
			if (ret == ~0 || ret == ~1)
			{
				if (retval)
					*retval = ~ret;
				break;
			}
			else
			{
				if (retval)
					*retval = ret;
			}
		}
	}

	return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerDisconnect(int playerid, int reason)
{
	server::player_pool.Remove(playerid);
	return true;
}
