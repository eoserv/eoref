#ifndef EO_DATA_HPP
#define EO_DATA_HPP

#include "data/pe_reader.hpp"

#include <map>
#include <memory>
#include <utility>
#include <vector>

// TODO: caching, other data file types

class Data
{
	public:
		struct EGF
		{
			int id;
			pe_reader pe;
			std::map<int, pe_reader::BitmapInfo> bitmap_table;
		};

		struct EGF_Graphic
		{
			EGF& egf;
			std::vector<char> bmp_data;
		};

		// first: egf id, second: graphic id
		using EGF_Graphic_Ref = std::pair<int, int>;

	private:
		// Data that is retained in memory
		std::vector<EGF> m_egfs;

		void load_all_data();

	public:
		std::shared_ptr<EGF_Graphic> bitmap(int egf_id, int id);

	friend void eo_init_data();
};

// Global data objects

extern Data* g_data;

// Load all data
void eo_init_data();

// Clean up data
void eo_destroy_data();

#endif // EO_DATA_HPP
