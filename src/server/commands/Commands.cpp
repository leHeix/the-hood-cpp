#include "../../main.hpp"

std::unique_ptr<std::unordered_map<std::string, command*>> commands::_commands;

cell commands::OnPlayerCommandText(std::uint16_t playerid, std::string cmdtext)
{
	if (!commands::_commands)
	{
		commands::_commands = std::make_unique<std::unordered_map<std::string, command*>>();
		return 1;
	}

	static const std::regex cmd_regex{ R"(^\/([\w\d]+)\s*(.*))" };
	std::smatch match;
	
	if (std::regex_match(cmdtext, match, cmd_regex))
	{
		std::string command_name = match[1];
		std::for_each(command_name.begin(), command_name.end(), [](char& c) { c = std::tolower(c); });

		if (commands::_commands->contains(command_name))
		{
			auto* player = server::player_pool[playerid];
			auto* command = commands::_commands->at(command_name);
			auto flags = static_cast<uint32_t>(command->_flags);

			if ((flags >> 16) > player->Rank())
				return ~1;

			if (!(flags & command::flags::no_cooldown) && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player->LastCommandTick()) < cmd::time_between_commands)
			{
				constexpr double commands_per_sec = (1000.0 / cmd::time_between_commands.count());
				if constexpr (commands_per_sec >= 1.0)
				{
					player->Chat()->Send(0xFFA02BFF, "Solo puedes enviar {} comando{} por segundo. Algunos comandos no disponen de tiempo de espera.", static_cast<int>(commands_per_sec), (commands_per_sec > 1.0 ? "s" : ""));
				}
				else
				{
					player->Chat()->Send(0xFFA02BFF, "Necesitas esperar {} segundos entre comando. Algunos comandos no disponen de tiempo de espera.", std::chrono::duration_cast<std::chrono::duration<double>>(cmd::time_between_commands).count());
				}
				
				return 1;
			}

			player->LastCommandTick() = std::chrono::steady_clock::now();

			std::string args;

			if (match[2].matched && match[2].length())
			{
				args = match[2];
			}
			
			argument_store st{ args };
			command->exec(server::player_pool[playerid], std::move(st));
		}

		return ~1;
	}

	return 1;
}

static CPublicHook<commands::OnPlayerCommandText> _cmd_opct("OnPlayerCommandText");

/*
template<typename CharT>
struct scn::scanner<CharT, CPlayer*> : scn::empty_parser
{
	template<typename Context>
	error scan(CPlayer*& val, Context& ctx)
	{
		std::uint16_t playerid;
		auto r = scn::scan(ctx.range(), "{}", playerid);
		if (!r)
		{
			std::string player_name;
			auto r = scn::scan(ctx.range(), "{}", player_name);
			if (!r)
				return scn::error(scn::error::invalid_scanned_value);

			for (auto&& [id, player] : server::player_pool)
			{
				if (player->Name().find(player_name) != std::string::npos)
				{
					val = player.get();
				}
			}
		}
		else
		{
			if (server::player_pool.Exists(playerid))
				val = server::player_pool[playerid];
		}

		ctx.range() = std::move(r.range());
		return r.error();
	}
};
*/