#pragma once

class CPlayerPool
{
	robin_hood::unordered_map<unsigned short, std::unique_ptr<CPlayer>> _players;

public:
	CPlayerPool() = default;
	~CPlayerPool() = default;

	CPlayer* Add(unsigned short id);
	void Remove(unsigned short id);
	void Remove(const CPlayer* player);

	inline bool Exists(unsigned short id) const
	{
		return _players.contains(id);
	}

	CPlayer* operator[](unsigned short id)
	{
		if (!_players.contains(id) && IsPlayerConnected(id))
			return Add(id);

		return _players.at(id).get();
	}
	CPlayer* Get(unsigned short id);

	// make it iterable
	auto begin() noexcept(noexcept(_players.begin())) { return _players.begin(); }
	auto end() noexcept(noexcept(_players.end())) { return _players.end(); }
};

namespace server 
{
	extern CPlayerPool player_pool;
}