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

	inline void nop(void* dest, size_t bytecount)
	{
		unlocked_scope lk(dest, bytecount);
		std::memset(dest, 0x90, bytecount);
	}

	namespace
	{
		constexpr uint32_t FNV_PRIME = 16777619U;
		constexpr uint32_t FNV_OFFSET_BASIS = 2166136261U;
	}

	constexpr std::uint32_t hash(const std::string_view str)
	{
		std::uint32_t h = FNV_OFFSET_BASIS;
		for (auto&& c : str)
		{
			h ^= c;
			h *= FNV_PRIME;
		}
		return h;
	}

	constexpr std::uint32_t hash(const char* str)
	{
		size_t len = std::char_traits<char>::length(str);

		std::size_t h = FNV_OFFSET_BASIS;
		for(size_t i = 0; i < len; ++i)
		{
			h ^= *str++;
			h *= FNV_PRIME;
		}

		return h;
	}

	std::size_t levenshtein(std::string s1, std::string s2, bool case_sensitive = false);
}

constexpr std::uint32_t operator""_hash(const char* s, size_t c)
{
	return utils::hash(s);
}
