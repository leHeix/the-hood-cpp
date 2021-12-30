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
		operator std::string() 
		{
			std::string result;
			cell* str_address;
			if (amx_GetAddr(amx, value, &str_address) != AMX_ERR_NONE)
				return std::move(result);

			int str_len{ 0 };
			amx_StrLen(str_address, &str_len);
			if (!str_len)
				return std::move(result);

			result.resize(++str_len);
			amx_GetString(result.data(), str_address, 0, str_len);
			result.pop_back();
			return std::move(result);
		}

		template<class T>
		operator T() {
			static_assert(std::is_convertible_v<cell, T>);
			return static_cast<T>(value);
		}

		AMX* amx;
		cell value;
	};

	struct public_hook
	{
		std::function<cell(AMX*, cell*)> call;

		template<class... Args>
		public_hook(cell(*fun)(Args...)) 
			:
				call([=,this](AMX* amx, cell* params) -> cell {
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

	extern std::unique_ptr<std::unordered_multimap<std::string, public_hook>> _hooks;
}

template<auto fun>
class CPublicHook
{
public:
	CPublicHook(const char* function_name)
	{
		if (!callbacks::_hooks)
		{
			std::cout << "[Public Hook] Created public hook store" << std::endl;
			callbacks::_hooks = std::make_unique<std::unordered_multimap<std::string, callbacks::public_hook>>();
		}

		std::cout << "[Public Hook] Registering hook to function " << function_name << " with address " << reinterpret_cast<void*>(fun) << std::endl;
		callbacks::_hooks->insert({ function_name, callbacks::public_hook(fun) });
	}
};