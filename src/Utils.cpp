#include "main.hpp"

void utils::FindAndReplace(std::string& source, const std::string_view find, const std::string_view replace)
{
	size_t i{ 0 };
	while ((i = source.find(find, i)) != std::string::npos)
	{
		source.replace(i, find.length(), replace);
		i += replace.length();
	}
}
