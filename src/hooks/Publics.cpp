#include "../main.hpp"

std::unique_ptr<std::unordered_multimap<std::string, callbacks::public_hook>> callbacks::_hooks{ nullptr };

PLUGIN_EXPORT bool PLUGIN_CALL OnPublicCall(AMX* amx, const char* name, cell* params, cell* retval)
{
    if (!callbacks::_hooks)
    {
        callbacks::_hooks = std::make_unique<std::unordered_multimap<std::string, callbacks::public_hook>>();
        return true;
    }

    // fixme: make an appropiate way to do this
    if (!strcmp("OnPlayerClickTextDraw", name) && params[2] == INVALID_TEXT_DRAW)
    {
        auto playerid = params[1];
        if (!server::player_pool[playerid]->_cancel_td_tick || std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - *server::player_pool[playerid]->_cancel_td_tick) < std::chrono::milliseconds{ 50 + GetPlayerPing(playerid) })
        {
            name = "OnPlayerCancelTextDrawSelection";
            params[0] = sizeof(cell);
        }

        server::player_pool[playerid]->_cancel_td_tick = std::nullopt;
        
        return true;
    }

    auto range = callbacks::_hooks->equal_range(name);
    for (auto it = range.first; it != range.second; ++it)
    {
        cell ret = it->second.call(amx, params);
        if (ret == ~0 || ret == ~1)
        {
            *retval = ~ret;
            break;
        }
        else
            *retval = ret;
    }

    return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerConnect(int playerid)
{
    return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerDisconnect(int playerid, int reason)
{
    return true;
}