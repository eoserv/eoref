#include "config.hpp"
#include "data.hpp"
#include "engine.hpp"
#include "game.hpp"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

int main()
{
	/* Allegro is required by the following 3 sub-systems:
	 *   - App (for windowing/input)
	 *   - Config (for config file loading)
	 *   - Engine_Allegro
	 */
	al_install_system(ALLEGRO_VERSION_INT, nullptr);

	al_init_image_addon();

	al_install_keyboard();
	al_install_mouse();

	eo_init_config();

	// The application also depends on these two global objects.
	eo_init_engine();
	eo_init_data();

	// App contains all of the logic to set up and run the client window
	{
		Game game;
		game.run();
	}

	eo_destroy_data();
	eo_destroy_engine();

	al_uninstall_mouse();
	al_uninstall_keyboard();

	al_shutdown_image_addon();
	al_uninstall_system();
}
