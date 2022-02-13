#include "../../main.hpp"

std::string commands::argument_store::GetNextChunk()
{
	if (_parsed == std::string::npos)
		throw std::runtime_error{ "string has already been parsed" };

	std::size_t space = std::string::npos;

	if (!_final)
	{
		space = _data.find(' ', _parsed);
		if (space != _data.length() - 1 && _data[space + 1] == ' ')
		{
			auto trailing = _data.find_first_not_of(' ', space);
			if (trailing == std::string::npos)
			{
				throw std::runtime_error{ "no more string to parse: trailing whitespaces" };
			}

			_parsed = trailing;
			space = _data.find(' ', _parsed);
		}
	}

	std::string chunk = _data.substr(_parsed, space - _parsed);

	_parsed = ++space;
	return std::move(chunk);
}

commands::argument_store& commands::argument_store::operator>>(int& data)
{
	std::string chunk = GetNextChunk();

	char* p;
	long val = std::strtol(chunk.c_str(), &p, 10);
	if (*p != '\0')
		throw type_error{ "integer", "not all data on chunk is integer" };

	data = val;
	++_parsed_arguments;
	return *this;
}

/*
commands::argument_store& commands::argument_store::operator>>(float& data)
{
	std::string chunk = GetNextChunk();

	char* p;
	float val = std::strtof(chunk.c_str(), &p);
	if (*p != '\0')
		throw type_error{ "floating-point number", "not all data on chunk is a floating-point number" };

	data = val;
	++_parsed_arguments;
	return *this;
}

commands::argument_store& commands::argument_store::operator>>(double& data)
{
	std::string chunk = GetNextChunk();

	char* p;
	float val = std::strtod(chunk.c_str(), &p);
	if (*p != '\0')
		throw type_error{ "floating-point number", "not all data on chunk is a floating-point number" };

	data = val;
	++_parsed_arguments;
	return *this;
}
*/

// Parsing strings is a bit more complicated since we need to also parse quoted strings
commands::argument_store& commands::argument_store::operator>>(std::string& data)
{
	data = GetNextChunk();
	if (data.starts_with('"'))
	{
		if (data.ends_with('"'))
		{
			data.erase(0, 1);
			data.pop_back();
		}
		else
		{
			_parsed -= data.length();
			auto closing = _data.find('"', _parsed);
			if (closing == std::string::npos)
			{
				data = _data.substr(_parsed);
				_parsed = std::string::npos;
			}
			else
			{
				data = _data.substr(_parsed, closing - _parsed);
				_parsed = closing + 2;
			}
		}
	}

	++_parsed_arguments;
	return *this;
}

commands::argument_store& commands::argument_store::operator>>(const final_t& f)
{
	_final = !_final;
	return *this;
}

commands::argument_store& commands::argument_store::operator>>(CPlayer*& data)
{
	std::string chunk = GetNextChunk();
	++_parsed_arguments;

	std::uint16_t playerid{};
	auto [ptr, ec] { std::from_chars(chunk.data(), chunk.data() + chunk.size(), playerid)};

	if (ec == std::errc{})
	{
		if (server::player_pool.Exists(playerid))
		{
			data = server::player_pool[playerid];
			return *this;
		}
	}

	// Name might be composed of integers but isn't a valid player ID.

	for (auto&& [id, player] : server::player_pool)
	{
		if (player->Name().find(chunk) != std::string::npos)
		{
			data = player.get();
			return *this;
		}
	}

	data = nullptr;
	return *this;
}

commands::argument_store& commands::argument_store::operator>>(vehicles::stModel& data)
{
	std::string chunk = GetNextChunk();

	std::uint16_t modelid{};
	auto [ptr, ec] { std::from_chars(chunk.data(), chunk.data() + chunk.size(), modelid)};
	if (ec == std::errc{})
	{
		if (modelid < 400 || modelid > 611)
		{
			throw type_error{ "vehicle model", "invalid vehicle model" };
		}

		data.id = modelid;
		data.name = vehicles::names[modelid - 400];
	}
	else
	{
		size_t min{ 8 };
		auto closest = vehicles::names.end();

		for (auto it = vehicles::names.begin(); it != vehicles::names.end(); ++it)
		{
			size_t distance = utils::levenshtein(chunk, std::string{ it->begin(), it->end() });
			if (distance < min)
			{
				min = distance;
				closest = it;
			}
		}

		if(closest == vehicles::names.end())
			throw type_error{ "vehicle model", "invalid vehicle model name" };

		data.id = 400 + std::distance(vehicles::names.begin(), closest);
		data.name = *closest;
	}

	return *this;
}

cmd::argument_store& cmd::argument_store::operator>>(CVehicle*& data)
{
	std::string chunk = GetNextChunk();

	std::uint16_t vehicleid{ INVALID_VEHICLE_ID };
	auto [ptr, ec] { std::from_chars(chunk.data(), chunk.data() + chunk.size(), vehicleid)};
	if (ec != std::errc{})
	{
		throw type_error{ "vehicle", "invalid vehicle ID" };
	}

	data = vehicles::vehicle_pool[vehicleid].get();
	return *this;
}