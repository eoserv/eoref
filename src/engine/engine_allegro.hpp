#ifndef EO_ENGINE_ALLEGRO_HPP
#define EO_ENGINE_ALLEGRO_HPP

#include "engine.hpp"

class Engine_Allegro : public Engine
{
	private:
		bool m_accel = false;

	public:
		Engine_Allegro();
		virtual ~Engine_Allegro();

		virtual Graphic create_texture(unsigned short width, unsigned short height);
		virtual Graphic load_bmp(const char* bmp_start, std::size_t bmp_size);

		virtual void render(Draw_Buffer&);
		virtual void render_to_target(Graphic& target, Draw_Buffer& draw_buffer);
};

#endif // EO_ENGINE_ALLEGRO_HPP
