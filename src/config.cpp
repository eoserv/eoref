#include "config.hpp"

#include "util/ascii.hpp"
#include "util/overload.hpp"

#include <allegro5/allegro.h>

#include <charconv>
#include <string_view>

// Default configuration settings
Config::Config()
	: Host("game.eoserv.net")
	, Port("8078")
	, Fullscreen(false)
	, Sizeable(false)
	, DrawEngine("allegro")
	, DrawAccel(true)
{ }

static ALLEGRO_CONFIG* g_al_config;

Config g_config;

static int parse_numeric(std::string_view str)
{
	namespace ascii = util::ascii;

	int value = 0;

	auto&& first = &str[0];
	auto&& last = first + str.size();

	auto result = std::from_chars(first, last, value);

	if (result.ec == std::errc{})
	{
		const char* p = result.ptr;

		while (p < last && ascii::isspace(*p))
		{
			++p;
		}

		if (p < last)
		{
			std::string_view unit_str(p);

			if (ascii::stricmp(unit_str, "kB") == 0 || ascii::stricmp(unit_str, "KiB") == 0)
				value *= 1024;
			else if (ascii::stricmp(unit_str, "MB") == 0 || ascii::stricmp(unit_str, "MiB") == 0)
				value *= 1024 * 1024;
			else if (ascii::stricmp(unit_str, "GB") == 0 || ascii::stricmp(unit_str, "GiB") == 0)
				value *= 1024 * 1024 * 1024;
		}
	}

	return value;
}

void eo_init_config()
{
	namespace ascii = util::ascii;

	enum section_t
	{
		section_unknown,
		section_connection,
		section_configuration,
		section_engine,
		section_settings,
		section_chat,
		section_language,
		section_custom
	} section = section_unknown;

	auto parse_config_entry = util::overload(
		[](const char*& out, std::string_view str)
		{
			out = str.data(); // value is null-terminated
		},
		[](bool& out, std::string_view str)
		{
			out = (ascii::stricmp(str, "on") == 0);
		},
		[](int& out, std::string_view str)
		{
			out = parse_numeric(str);
		}
	);

	if (g_al_config)
		return;

	g_al_config = al_load_config_file("config/setup.ini");

	ALLEGRO_CONFIG_SECTION* section_it;
	const char* section_name = al_get_first_config_section(g_al_config, &section_it);

	for (; section_it; section_name = al_get_next_config_section(&section_it))
	{
		std::string_view section_name_str(section_name);

		section =
			  (ascii::stricmp(section_name_str, "CONNECTION")) == 0    ? section_connection
			: (ascii::stricmp(section_name_str, "CONFIGURATION")) == 0 ? section_configuration
			: (ascii::stricmp(section_name_str, "ENGINE")) == 0        ? section_engine
			: (ascii::stricmp(section_name_str, "SETTINGS")) == 0      ? section_settings
			: (ascii::stricmp(section_name_str, "CHAT")) == 0          ? section_chat
			: (ascii::stricmp(section_name_str, "LANGUAGE")) == 0      ? section_language
			: (ascii::stricmp(section_name_str, "CUSTOM")) == 0        ? section_custom
			: section_unknown;

		ALLEGRO_CONFIG_ENTRY* entry_it;
		const char* entry_name = al_get_first_config_entry(g_al_config, section_name, &entry_it);

		for (; entry_it; entry_name = al_get_next_config_entry(&entry_it))
		{
			const char* entry = al_get_config_value(g_al_config, section_name, entry_name);
			std::string_view entry_name_str(entry_name);

			switch (section)
			{
				case section_connection:
					if (ascii::stricmp(entry_name_str, "Host") == 0)
						parse_config_entry(g_config.Host, entry);
					else if (ascii::stricmp(entry_name_str, "Port") == 0)
						parse_config_entry(g_config.Port, entry);
					break;

				case section_configuration:
					if (ascii::stricmp(entry_name_str, "Fullscreen") == 0)
						parse_config_entry(g_config.Fullscreen, entry);
					else if (ascii::stricmp(entry_name_str, "Sizeable") == 0)
						parse_config_entry(g_config.Sizeable, entry);
					break;

				case section_engine:
					if (ascii::stricmp(entry_name_str, "DrawEngine") == 0)
						parse_config_entry(g_config.DrawEngine, entry);
					else if (ascii::stricmp(entry_name_str, "DrawAccel") == 0)
						parse_config_entry(g_config.DrawAccel, entry);
					break;

				case section_settings:
					// TODO
					break;

				case section_chat:
					// TODO
					break;

				case section_language:
					// TODO
					break;

				case section_custom:
					// TODO
					break;

				case section_unknown:
					break;
			}
		}
	}
}
