#include "../main.hpp"

CPlayer* CPlayerPool::Add(unsigned short id)
{
	if (_players.contains(id))
		return _players[id].get();

	sampgdk::logprintf("[PlayerPool] Added player %i", id);

	_players[id] = std::make_unique<CPlayer>(id);
	return _players[id].get();
}

void CPlayerPool::Remove(unsigned short id)
{
	_players.erase(id);
}

void CPlayerPool::Remove(const CPlayer* player)
{
	_players.erase(player->PlayerId());
}

CPlayer* CPlayerPool::Get(unsigned short id)
{
	try
	{
		return _players.at(id).get();
	}
	catch (const std::out_of_range& e)
	{
		return nullptr;
	}
}

CPlayerPool server::player_pool{};