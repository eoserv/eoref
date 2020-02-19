#ifndef EO_DATA_HPP
#define EO_DATA_HPP

#include "eo_pub_protocol.hpp"

#include <map>
#include <optional>

// Extends the auto-generated EMF_File with code to read the tile data
struct Full_EMF
{
	EMF_File header;

	struct TileMeta
	{
		std::optional<EMF_Tile_Spec> spec;
		std::optional<EMF_Warp> warp;
		std::optional<std::pair<std::string, std::string>> sign;
	};

	std::vector<TileMeta> m_meta;
	std::vector<std::array<eo_short, 9>> m_gfx;

	TileMeta& meta(int x, int y)
		{ return m_meta[y * header.width + x]; }

	std::array<eo_short, 9>& gfx(int x, int y)
		{ return m_gfx[y * header.width + x]; }

	const TileMeta& meta(int x, int y) const
		{ return m_meta[y * header.width + x]; }

	const std::array<eo_short, 9>& gfx(int x, int y) const
		{ return m_gfx[y * header.width + x]; }

	static void decode_str(char* data, size_t n);

	Full_EMF() = default;
	Full_EMF(EO_Stream_Reader& reader) { unserialize(reader); }
	std::size_t byte_size() const;
	void serialize(EO_Stream_Builder& builder) const;
	void unserialize(EO_Stream_Reader& reader);
};

#endif // EO_DATA_HPP
