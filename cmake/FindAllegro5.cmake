
find_path(ALLEGRO5_INCLUDE_DIR
	NAMES allegro5/allegro.h
)

find_library(ALLEGRO5_LIBRARY
	NAMES allegro allegro-static
)

find_library(ALLEGRO5_DIALOG_LIBRARY NAMES allegro_dialog allegro_dialog-static)
find_library(ALLEGRO5_IMAGE_LIBRARY NAMES allegro_image allegro_image-static)
find_library(ALLEGRO5_PRIMITIVES_LIBRARY NAMES allegro_primitives allegro_primitives-static)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	Allegro5 FOUND_VAR ALLEGRO5_FOUND
	REQUIRED_VARS ALLEGRO5_INCLUDE_DIR ALLEGRO5_LIBRARY
	ALLEGRO5_DIALOG_LIBRARY ALLEGRO5_IMAGE_LIBRARY ALLEGRO5_PRIMITIVES_LIBRARY
)

mark_as_advanced(
	ALLEGRO5_INCLUDE_DIR
	ALLEGRO5_LIBRARY
	ALLEGRO5_DIALOG_LIBRARY
	ALLEGRO5_IMAGE_LIBRARY
	ALLEGRO5_PRIMITIVES_LIBRARY
)

