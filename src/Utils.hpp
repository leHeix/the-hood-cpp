#pragma once

#define IO_GETTER_SETTER(name,field) inline auto& name() { return field; } inline const auto& name() const { return field; }

namespace utils
{
	void FindAndReplace(std::string& source, const std::string_view find, const std::string_view replace);
	template<class T = int>
	T Random(T min, T max)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<T> dist(min, max);
		return dist(mt);
	}
}