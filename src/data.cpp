#include "data.hpp"

#include "cio/cio.hpp"
#include "data/pe_reader.hpp"
#include "util/int_pack.hpp"

#include "trace.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <vector>

#define TRACE_CTX "data"

static constexpr int EGF_MAX = 25;

void Data::load_all_data()
{
	m_egfs.reserve(EGF_MAX);

	for (int id = 1; id <= EGF_MAX; ++id)
	{
		auto&& fname = fmt::format("gfx/gfx{:03}.egf", id);
		cio::stream f(fname.c_str());

		if (f)
		{
			auto&& pe = pe_reader(std::move(f));
			pe.read_header();

			auto&& bmp_table = pe.read_bitmap_table();
			trace_log("EGF " << id << " table size " << bmp_table.size());
			auto&& egf = EGF{id, std::move(pe), std::move(bmp_table)};

			m_egfs.push_back(std::move(egf));
		}
	}
}

std::shared_ptr<Data::EGF_Graphic> Data::bitmap(int egf_id, int id)
{
	if (egf_id > EGF_MAX)
		return nullptr;

	auto&& egf = m_egfs[egf_id - 1];
	auto&& bmp_info_it = egf.bitmap_table.find(id);

	if (bmp_info_it == egf.bitmap_table.end())
	{
		trace_log("bmp " << egf_id << "/" << id << " not found");
		return nullptr;
	}

	auto&& bmp_info = bmp_info_it->second;

	if (bmp_info.size < 2 || bmp_info.size > 0xFFFFFFFFU - 14)
	{
		trace_log("bmp size out of range");
		return nullptr;
	}

	char dib_header_size_bytes[2];

	if (!egf.pe.read_resource(&dib_header_size_bytes[0], bmp_info.start, 2))
	{
		trace_log("bmp header read failed");
		return nullptr;
	}

	auto&& dib_header_size = util::int_pack_16_le(dib_header_size_bytes);

	auto&& bmp_size_bytes = util::int_unpack_32_le(bmp_info.size + 14);
	auto&& pixel_offset_bytes = util::int_unpack_32_le(dib_header_size + 14);

	std::vector<char> bmp_buf;
	bmp_buf.resize(bmp_info.size + 14, '\0');
	std::copy_n("BM", 2, &bmp_buf[0]);
	std::copy_n(&bmp_size_bytes[0], 4, &bmp_buf[2]);
	std::copy_n(&pixel_offset_bytes[0], 4, &bmp_buf[10]);

	if (!egf.pe.read_resource(&bmp_buf[14], bmp_info.start, bmp_info.size))
	{
		trace_log("bmp read failed");
		return nullptr;
	}

	auto&& graphic = EGF_Graphic{
		egf, std::move(bmp_buf)
	};

	return std::make_shared<EGF_Graphic>(graphic);
}

// Global object

Data* g_data;

void eo_init_data()
{
	if (g_data)
		return;

	g_data = new Data();
	g_data->load_all_data();
}

void eo_destroy_data()
{
	if (g_data)
		delete g_data;

	g_data = nullptr;
}
