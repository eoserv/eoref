#ifndef EO_GFX_COLOR_HPP
#define EO_GFX_COLOR_HPP

#include "util/int_pack.hpp"

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <utility>

struct Color
{
	// RGBA
	std::uint32_t m_color = 0;

	explicit constexpr Color(std::uint32_t color = 0) noexcept
		: m_color(color)
	{ }

	constexpr Color(std::uint8_t r, std::uint8_t g,
	                std::uint8_t b, std::uint8_t a = 0xFF)  noexcept
		: m_color(util::int_pack_32_be(r, g, b, a))
	{ }

	constexpr std::uint8_t r() const noexcept { return (m_color & 0xFF000000U) >> 24; }
	constexpr std::uint8_t g() const noexcept { return (m_color & 0x00FF0000U) >> 16; }
	constexpr std::uint8_t b() const noexcept { return (m_color & 0x0000FF00U) >>  8; }
	constexpr std::uint8_t a() const noexcept { return (m_color & 0x000000FFU)      ; }

	constexpr void r(std::uint8_t val) noexcept
	{
		m_color = (m_color & 0x00FFFFFFU) | (std::uint32_t(val) << 24);
	}

	constexpr void g(std::uint8_t val) noexcept
	{
		m_color = (m_color & 0xFF00FFFFU) | (std::uint32_t(val) << 16);
	}

	constexpr void b(std::uint8_t val) noexcept
	{
		m_color = (m_color & 0xFFFF00FFU) | (std::uint32_t(val) << 8);
	}

	constexpr void a(std::uint8_t val) noexcept
	{
		m_color = (m_color & 0xFFFFFF00U) | std::uint32_t(val);
	}

	explicit constexpr operator std::uint32_t() noexcept
	{
		return m_color;
	}
};

static constexpr Color rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
{
	return Color(r, g, b);
}

static constexpr Color rgba(std::uint8_t r, std::uint8_t g,
                     std::uint8_t b, std::uint8_t a) noexcept
{
	return Color(r, g, b, a);
}

static constexpr bool operator==(const Color& a, const Color& b) noexcept
{
	return (a.m_color == b.m_color);
}

static constexpr bool operator!=(const Color& a, const Color& b) noexcept
{
	return (a.m_color != b.m_color);
}

static constexpr bool operator<(const Color& a, const Color& b) noexcept
{
	return (a.m_color < b.m_color);
}

static constexpr bool operator<=(const Color& a, const Color& b) noexcept
{
	return (a.m_color <= b.m_color);
}

static constexpr bool operator>(const Color& a, const Color& b) noexcept
{
	return (a.m_color > b.m_color);
}

static constexpr bool operator>=(const Color& a, const Color& b) noexcept
{
	return (a.m_color > b.m_color);
}

static void swap(Color& a, Color& b) noexcept
{
	std::swap(a.m_color, b.m_color);
}

namespace std
{
	template <>
	struct hash<Color>
	{
		std::size_t operator()(const Color& c) noexcept
		{
			return std::size_t(c.m_color);
		}
	};
}

#endif // EO_GFX_COLOR_HPP
