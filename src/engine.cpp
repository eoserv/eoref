#include "engine.hpp"

#include "engine/engine_allegro.hpp"

Engine::~Engine()
{ }

// Global object

Engine* g_engine;

void eo_init_engine()
{
	if (g_engine)
		return;

	g_engine = new Engine_Allegro();
}

void eo_destroy_engine()
{
	if (g_engine)
		delete g_engine;

	g_engine = nullptr;
}
