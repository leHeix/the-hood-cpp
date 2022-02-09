#pragma once

class command;

namespace commands
{
	extern std::unique_ptr<std::unordered_map<std::string, command*>> _commands;
	constexpr inline std::chrono::milliseconds time_between_commands = std::chrono::milliseconds{ 1000 };
}

namespace cmd = commands;

class command
{
public:
	enum flags : std::int32_t
	{
		hidden = 1 << 0,
		no_cooldown = 1 << 1,

		max_flag
	};
	static_assert(static_cast<int>(flags::max_flag) < 16777216 + 1);
	template<std::uint8_t Rank> static inline constexpr flags make_flag = static_cast<flags>(Rank << 24);

private:
	void Register(const std::string_view name)
	{
		if (!commands::_commands)
		{
			commands::_commands = std::make_unique<std::unordered_map<std::string, command*>>();
			std::cout << "[Commands] Created command store" << std::endl;
		}

		std::string name_lower(name);
		std::for_each(name_lower.begin(), name_lower.end(), [](char& c) { c = std::tolower(c); });

		commands::_commands->insert({ name_lower, this });
		std::cout << "[Commands] Registered command " << std::quoted(name_lower) << std::endl;
	}

	flags _flags{ 0u };
public:
	std::function<void(CPlayer*, commands::argument_store)> exec;

	command(const std::string_view name, std::function<void(CPlayer*, commands::argument_store)> fun)
		: exec(fun)
	{
		Register(name);
	}

	command(const std::string_view name, std::initializer_list<const std::string_view> aliases, std::function<void(CPlayer*, commands::argument_store)> fun)
		: exec(fun)
	{
		Register(name);
		
		for (auto&& alias : aliases)
		{
			std::string alias_lowered{ alias };
			std::for_each(alias_lowered.begin(), alias_lowered.end(), [](char& c) { c = std::tolower(c); });
			commands::_commands->insert({ alias_lowered, this });
			std::cout << "[Commands] Registered alias " << std::quoted(alias_lowered) << " to " << std::quoted(name) << std::endl;
		}
	}

	command(const std::string_view name, flags flags, std::function<void(CPlayer*, commands::argument_store)> fun)
		: _flags(flags), exec(fun)
	{
		Register(name);
	}

	command(const std::string_view name, flags flags, std::initializer_list<const std::string_view> aliases, std::function<void(CPlayer*, commands::argument_store)> fun)
		: _flags(flags), exec(fun)
	{
		Register(name);

		for (auto&& alias : aliases)
		{
			std::string alias_lowered{ alias };
			std::for_each(alias_lowered.begin(), alias_lowered.end(), [](char& c) { c = std::tolower(c); });
			commands::_commands->insert({ alias_lowered, this });
			std::cout << "[Commands] Registered alias " << std::quoted(alias_lowered) << " to " << std::quoted(name) << std::endl;
		}
	}

	command(const std::string_view name, std::initializer_list<const std::string_view> aliases, flags flags, std::function<void(CPlayer*, commands::argument_store)> fun)
		: _flags(flags), exec(fun)
	{
		Register(name);

		for (auto&& alias : aliases)
		{
			std::string alias_lowered{ alias };
			std::for_each(alias_lowered.begin(), alias_lowered.end(), [](char& c) { c = std::tolower(c); });
			commands::_commands->insert({ alias_lowered, this });
			std::cout << "[Commands] Registered alias " << std::quoted(alias_lowered) << " to " << std::quoted(name) << std::endl;
		}
	}

	IO_GETTER_SETTER(Flags, _flags)
};
