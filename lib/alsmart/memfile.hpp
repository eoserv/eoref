#ifndef ALSMART_MEMFILE_HPP
#define ALSMART_MEMFILE_HPP

#include "alsmart.hpp"

#include <allegro5/allegro_memfile.h>

namespace alsmart
{
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FILE, open_memfile)
}

#endif // ALSMART_MEMFILE_HPP
