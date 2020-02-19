#ifndef EO_ASCII_HPP
#define EO_ASCII_HPP

#include <algorithm>
#include <string_view>

namespace util::ascii
{
	constexpr char tolower(char c) noexcept
	{
		if (c >= 0x41 && c <= 0x5A)
			return char(c + 0x20);
		else
			return c;
	}

	constexpr char toupper(char c) noexcept
	{
		if (c >= 0x61 && c <= 0x7A)
			return char(c - 0x20);
		else
			return c;
	}

	constexpr bool chrcmp(char a, char b) noexcept
	{
		return tolower(a) == tolower(b);
	}

	constexpr bool isspace(char c) noexcept
	{
		return (c == 0x20 || c == 0x09 || c == 0x0A
		     || c == 0x0B || c == 0x0C || c == 0x0D);
	}

	constexpr int stricmp(std::string_view a, std::string_view b) noexcept
	{
		auto&& a_size = a.size();
		auto&& b_size = b.size();

		auto length = a.size();

		if (b.size() < length)
			length = b.size();

		for (std::string_view::size_type i = 0; i < length; ++i)
		{
			auto&& char_a = a[i];
			auto&& char_b = b[i];

			if (char_a < char_b)
				return -1;

			if (char_a > char_b)
				return 1;
		}

		if (a_size < b_size)
			return -1;

		if (a_size > b_size)
			return 1;

		return 0;
	}
}

#endif // EO_UTIL_ASCII_HPP
