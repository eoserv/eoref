#ifndef ALSMART_FONT_HPP
#define ALSMART_FONT_HPP

#include "alsmart.hpp"

#include <allegro5/allegro_font.h>

namespace alsmart
{
	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_FONT, font, al_destroy_font)

	// ---

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FONT, load_font)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FONT, grab_font_from_bitmap)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FONT, load_bitmap_font)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FONT, load_bitmap_font_flags)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FONT, create_builtin_font)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FONT, load_ttf_font_stretch)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FONT, load_ttf_font_stretch_f)
}

#endif // ALSMART_FONT_HPP
