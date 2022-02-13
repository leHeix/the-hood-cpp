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

std::size_t utils::levenshtein(std::string s1, std::string s2, bool case_sensitive)
{
	if (case_sensitive)
	{
		std::for_each(s1.begin(), s1.end(), [](char& c) { c = std::tolower(c); });
		std::for_each(s2.begin(), s2.end(), [](char& c) { c = std::tolower(c); });
	}

	const size_t m = s1.size();
	const size_t n = s2.size();

	if (m == 0)
		return n;

	if (n == 0)
		return m;

	std::vector<std::size_t> costs(n + 1);
	std::iota(costs.begin(), costs.end(), 0);
	std::size_t i{ 0 };

	for (auto&& c1 : s1)
	{
		costs[0] = i + 1;
		std::size_t corner = i;
		std::size_t j = 0;

		for (auto&& c2 : s2)
		{
			std::size_t upper = costs[j + 1];
			costs[j + 1] = (c1 == c2) ? corner
				: 1 + std::min(std::min(upper, corner), costs[j]);
			corner = upper;
			++j;
		}

		++i;
	}

	return costs[n];
}