#ifndef EO_UTIL_INT_PACK_HPP
#define EO_UTIL_INT_PACK_HPP

#include <array>
#include <cstdint>

namespace util
{
	constexpr std::uint16_t int_pack_16_le(std::uint8_t a, std::uint8_t b)
	{
		const std::uint16_t x[] = {a, b};

		return static_cast<std::uint16_t>(
			(x[0]) | (x[1] << 8)
		);
	}

	constexpr std::uint32_t int_pack_32_le(std::uint8_t a, std::uint8_t b,
										   std::uint8_t c, std::uint8_t d)
	{
		const std::uint32_t x[] = {a, b, c, d};

		return static_cast<std::uint32_t>(
			(x[0]) | (x[1] << 8) | (x[2] << 16) | (x[3] << 24)
		);
	}

	constexpr std::uint64_t int_pack_64_le(std::uint8_t a, std::uint8_t b,
										   std::uint8_t c, std::uint8_t d,
										   std::uint8_t e, std::uint8_t f,
										   std::uint8_t g, std::uint8_t h)
	{
		const std::uint64_t x[] = {a, b, c, d, e, f, g, h};

		return static_cast<std::uint64_t>(
			(x[0]      ) | (x[1] <<  8) | (x[2] << 16) | (x[3] << 24) |
			(x[4] << 32) | (x[5] << 40) | (x[6] << 48) | (x[7] << 56)
		);
	}

	// ---

	constexpr std::uint16_t int_pack_16_be(std::uint8_t a, std::uint8_t b)
	{
		const std::uint16_t x[] = {a, b};

		return static_cast<std::uint16_t>(
			(x[0] << 8) | (x[1])
		);
	}

	constexpr std::uint32_t int_pack_32_be(std::uint8_t a, std::uint8_t b,
										   std::uint8_t c, std::uint8_t d)
	{
		const std::uint32_t x[] = {a, b, c, d};

		return static_cast<std::uint32_t>(
			(x[0] << 24) | (x[1] << 16) | (x[2] << 8) | (x[3])
		);
	}

	constexpr std::uint64_t int_pack_64_be(std::uint8_t a, std::uint8_t b,
										   std::uint8_t c, std::uint8_t d,
										   std::uint8_t e, std::uint8_t f,
										   std::uint8_t g, std::uint8_t h)
	{
		const std::uint64_t x[] = {a, b, c, d, e, f, g, h};

		return static_cast<std::uint64_t>(
			(x[0] << 56) | (x[1] << 48) | (x[2] << 40) | (x[3] << 32) |
			(x[4] << 24) | (x[5] << 16) | (x[6] <<  8) | (x[7])
		);
	}

	// ---

	inline std::uint16_t int_pack_16_le(const char* bytes)
	{
		return int_pack_16_le(bytes[0], bytes[1]);
	}

	inline std::uint32_t int_pack_32_le(const char* bytes)
	{
		return int_pack_32_le(bytes[0], bytes[1], bytes[2], bytes[3]);
	}

	inline std::uint64_t int_pack_64_le(const char* bytes)
	{
		return int_pack_64_le(bytes[0], bytes[1], bytes[2], bytes[3],
							  bytes[4], bytes[5], bytes[6], bytes[7]);
	}

	// ---

	inline std::uint16_t int_pack_16_be(const char* bytes)
	{
		return int_pack_16_be(bytes[0], bytes[1]);
	}

	inline std::uint32_t int_pack_32_be(const char* bytes)
	{
		return int_pack_32_be(bytes[0], bytes[1], bytes[2], bytes[3]);
	}

	inline std::uint64_t int_pack_64_be(const char* bytes)
	{
		return int_pack_64_be(bytes[0], bytes[1], bytes[2], bytes[3],
							  bytes[4], bytes[5], bytes[6], bytes[7]);
	}

	// ---

	constexpr std::array<std::uint8_t, 2> int_unpack_16_le(std::uint16_t x)
	{
		return {{
			std::uint8_t((x     ) & 0xFF),
			std::uint8_t((x >> 8) & 0xFF)
		}};
	}

	constexpr std::array<std::uint8_t, 4> int_unpack_32_le(std::uint32_t x)
	{
		return {{
			std::uint8_t((x      ) & 0xFF),
			std::uint8_t((x >>  8) & 0xFF),
			std::uint8_t((x >> 16) & 0xFF),
			std::uint8_t((x >> 24) & 0xFF)
		}};
	}

	constexpr std::array<std::uint8_t, 8> int_unpack_64_le(std::uint64_t x)
	{
		return {{
			std::uint8_t((x      ) & 0xFF),
			std::uint8_t((x >>  8) & 0xFF),
			std::uint8_t((x >> 16) & 0xFF),
			std::uint8_t((x >> 24) & 0xFF),
			std::uint8_t((x >> 32) & 0xFF),
			std::uint8_t((x >> 40) & 0xFF),
			std::uint8_t((x >> 48) & 0xFF),
			std::uint8_t((x >> 56) & 0xFF)
		}};
	}

	// ---

	constexpr std::array<std::uint8_t, 2> int_unpack_16_be(std::uint16_t x)
	{
		return {{
			std::uint8_t((x >> 8) & 0xFF),
			std::uint8_t((x     ) & 0xFF)
		}};
	}

	constexpr std::array<std::uint8_t, 4> int_unpack_32_be(std::uint32_t x)
	{
		return {{
			std::uint8_t((x >> 24) & 0xFF),
			std::uint8_t((x >> 16) & 0xFF),
			std::uint8_t((x >>  8) & 0xFF),
			std::uint8_t((x      ) & 0xFF)
		}};
	}

	constexpr std::array<std::uint8_t, 8> int_unpack_64_be(std::uint64_t x)
	{
		return {{
			std::uint8_t((x >> 56) & 0xFF),
			std::uint8_t((x >> 48) & 0xFF),
			std::uint8_t((x >> 40) & 0xFF),
			std::uint8_t((x >> 32) & 0xFF),
			std::uint8_t((x >> 24) & 0xFF),
			std::uint8_t((x >> 16) & 0xFF),
			std::uint8_t((x >>  8) & 0xFF),
			std::uint8_t((x      ) & 0xFF)
		}};
	}
}

#endif // EO_UTIL_INT_PACK_HPP
