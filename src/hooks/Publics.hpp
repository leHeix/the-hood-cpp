#pragma once

namespace callbacks
{
	struct castable_cell
	{
		operator cell() { return value; }
		operator cell* () 
		{
			cell* address{ nullptr };
			if (amx_GetAddr(amx, value, &address) != AMX_ERR_NONE)
				return nullptr;

			return address;
		}
		operator float() { return amx_ctof(value); }
		operator float* () { return reinterpret_cast<float*>(static_cast<cell*>(*this)); }
		operator std::string() 
		{
			std::string result;
			cell* str_address;
			if (amx_GetAddr(amx, value, &str_address) != AMX_ERR_NONE)
				return result;

			int str_len{ 0 };
			amx_StrLen(str_address, &str_len);
			if (!str_len)
				return result;

			result.resize(++str_len);
			amx_GetString(result.data(), str_address, 0, str_len);
			result.pop_back();

			return result;
		}

		template<class T>
		operator T() {
			static_assert(std::is_convertible_v<T, cell>);
			return static_cast<T>(value);
		}

		AMX* amx;
		cell value;
	};

	struct hook
	{
		std::function<cell(AMX*, cell*)> call;

		template<class... Args>
		hook(const std::function<cell(Args...)>& fun)
			:
				call([=](AMX* amx, cell* params) -> cell {
					if (params[0] / sizeof(cell) != sizeof...(Args))
					{
						sampgdk::logprintf("[Public Hook] Error while converting parameters: expected %i arguments, got %i.", sizeof...(Args), params[0] / sizeof(cell));
						return 0;
					}

					auto unpack = [=]<std::size_t... idx>(std::index_sequence<idx...>) -> cell
					{
						return fun(castable_cell{ amx, params[idx + 1] }...);
					};

					return unpack(std::make_index_sequence<sizeof...(Args)>{});
				})
		{

		}
	};

	extern std::unique_ptr<std::unordered_multimap<std::string, hook>> _prehooks;
	extern std::unique_ptr<std::unordered_multimap<std::string, hook>> _hooks;
	extern std::unique_ptr<std::unordered_multimap<std::string, hook>> _posthooks;
}

class public_hook {
public:
	template<class F>
	public_hook(const char* function_name, F&& fun)
	{
		// https://stackoverflow.com/questions/59356874
		auto function = std::function{ std::forward<F>(fun) };

		if (!callbacks::_hooks)
		{
			std::cout << "[Public Hook] Created public hook store" << std::endl;
			callbacks::_hooks = std::make_unique<std::unordered_multimap<std::string, callbacks::hook>>();
		}

		std::cout << "[Public Hook] Registering hook to function " << std::quoted(function_name) <<  std::endl;
		callbacks::_hooks->insert({ function_name, callbacks::hook(function) });
	}

	public_hook(const public_hook&) = delete;
	public_hook(public_hook&&) = delete;
};

class public_prehook {
public:
	template<class F>
	public_prehook(const char* function_name, F&& fun)
	{
		// https://stackoverflow.com/questions/59356874
		auto function = std::function{ std::forward<F>(fun) };

		if (!callbacks::_prehooks)
		{
			std::cout << "[Public Hook] Created public prehook store" << std::endl;
			callbacks::_prehooks = std::make_unique<std::unordered_multimap<std::string, callbacks::hook>>();
		}

		std::cout << "[Public Hook] Registering prehook to function " << std::quoted(function_name) << std::endl;
		callbacks::_prehooks->insert({ function_name, callbacks::hook(function) });
	}

	public_prehook(const public_prehook&) = delete;
	public_prehook(public_prehook&&) = delete;
};

class public_posthook {
public:
	template<class F>
	public_posthook(const char* function_name, F&& fun)
	{
		// https://stackoverflow.com/questions/59356874
		auto function = std::function{ std::forward<F>(fun) };

		if (!callbacks::_posthooks)
		{
			std::cout << "[Public Hook] Created public posthook store" << std::endl;
			callbacks::_posthooks = std::make_unique<std::unordered_multimap<std::string, callbacks::hook>>();
		}

		std::cout << "[Public Hook] Registering posthook to function " << std::quoted(function_name) << std::endl;
		callbacks::_posthooks->insert({ function_name, callbacks::hook(function) });
	}

	public_posthook(const public_prehook&) = delete;
	public_posthook(public_prehook&&) = delete;
};

template<auto fun>
class CPublicHook
{
public:
	CPublicHook(const char* function_name)
	{
		if (!callbacks::_hooks)
		{
			std::cout << "[Public Hook] Created public hook store" << std::endl;
			callbacks::_hooks = std::make_unique<std::unordered_multimap<std::string, callbacks::hook>>();
		}

		std::cout << "[Public Hook] Registering hook to function " << function_name << " with address " << reinterpret_cast<void*>(fun) << std::endl;
		callbacks::_hooks->insert({ function_name, callbacks::hook(std::function{ fun }) });
	}
};

// to-do: post-hooks
