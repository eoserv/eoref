#ifndef EO_TYPES_HPP
#define EO_TYPES_HPP

#include <array>

using eo_byte = unsigned char;
using eo_char = unsigned char;
using eo_short = unsigned short;
using eo_three = int;
using eo_int = int;

constexpr unsigned eo_number_decode(unsigned a = 254, unsigned b = 254,
                                    unsigned c = 254, unsigned d = 254)
{
	if (a == 254U) a = 1;
	if (b == 254U) b = 1;
	if (c == 254U) c = 1;
	if (d == 254U) d = 1;

	--a;
	--b;
	--c;
	--d;

	return (d * 253U * 253U * 253U)
	     + (c * 253U * 253U)
	     + (b * 253U)
	     +  a;
}

constexpr std::array<eo_byte, 4> eo_encode_number(unsigned n)
{
	eo_byte a = 254U, b = 254U, c = 254U, d = 254U;

	{
		constexpr unsigned max = 253U * 253U * 253U;
		d = (n / max) + 1;
		n %= max;
	}

	{
		constexpr unsigned max = 253U * 253U;
		c = (n / max) + 1;
		n %= max;
	}

	{
		constexpr unsigned max = 253U;
		b = (n / max) + 1;
		n %= max;
	}

	// Should do this maybe for correctness:
	//  if (x == 1) x = 254;

	a = n + 1;

	return {a, b, c, d};
}

#endif // EO_TYPES_HPP

