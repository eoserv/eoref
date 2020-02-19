#ifndef EO_CONFIG_HPP
#define EO_CONFIG_HPP

// Unsupported / unused configuration values are commented out
struct Config
{
	// [CONNECTION]
	const char* Host;
	const char* Port;

	// [CONFIGURATION]
	bool Fullscreen;
	//bool ExclusiveFullscreen;
	bool Sizeable;

	// [ENGINE]
	const char* DrawEngine;
	bool DrawAccel;
	//int MaxFPS;

	// [SETTINGS]
	//bool Music;
	//bool Sound;
	//bool ShowBaloons;
	//bool ShowShadows;

	// [CHAT]
	//bool Filter;
	//bool FilterAll;
	//bool LogChat;
	//const char* LogFile;
	//bool HearWhisper;
	//bool Interaction;

	// [LANGUAGE]
	//int Language;

	// [CUSTOM]
	//bool Skipintro;
	//bool Skipsoftpad;

	Config();
};

// Global object

extern Config g_config;

// Fill out g_config from setup.ini
extern void eo_init_config();

#endif // EO_CONFIG_HP
