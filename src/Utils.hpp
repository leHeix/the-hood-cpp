#pragma once

#define IO_GETTER_SETTER(name,field) inline auto& name() { return field; } inline const auto& name() const { return field; }

#ifdef _WIN32
	#define ALIGN(x)
#else
	#define ALIGN(addr) (char *)(((long)addr + sysconf(_SC_PAGESIZE)-1) & ~(sysconf(_SC_PAGESIZE)-1))
#endif

namespace utils
{
	void FindAndReplace(std::string& source, const std::string_view find, const std::string_view replace);

	class unlocked_scope {
#ifdef _WIN32
		DWORD _oldflags;
#endif
		void* _address;
		std::size_t _size;
	public:
		unlocked_scope(void* address, std::size_t size)
			: _address(address), _size(size)
		{
#if _WIN32
			VirtualProtect(_address, _size, PAGE_EXECUTE_READWRITE, &_oldflags);
#else
			_address = ALIGN(_address);
			mprotect(_address, _size, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
		}

		~unlocked_scope()
		{
#ifdef _WIN32
			VirtualProtect(_address, _size, _oldflags, &_oldflags);
#else
			mprotect(_address, _size, PROT_READ | PROT_EXEC);
#endif
		}
	};
}

namespace server 
{
	/*
	namespace
	{
		template<class A>
		struct ctype { using type = A; };
		struct ctype<std::string> { using type = const char*; };
		struct ctype<std::string_view> { using type = const char*; };

		template<class T>
		static auto convert_to_ctype(const T& value) -> ctype<std::remove_cvref_t<T>>::type
		{
			using PlainT = std::remove_cvref_t<T>;
			if constexpr (std::is_same_v<PlainT, std::string>)
			{
				return value.c_str();
			}
			else if constexpr (std::is_same_v<PlainT, std::string_view>)
			{
				return value.data();
			}

			return value;
		}
	}
	*/
}