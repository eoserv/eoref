#include "engine_allegro.hpp"

#include "gfx/draw_buffer_debug.hpp"
#include "config.hpp"

#include "alsmart/alsmart.hpp"
#include "alsmart/memfile.hpp"

#include <allegro5/allegro_primitives.h>
#include <stack>

namespace
{
	ALLEGRO_COLOR al_col(const Color& c)
	{
		return al_map_rgba(c.r(), c.g(), c.b(), c.a());
	}

	ALLEGRO_BITMAP* al_bmp(Engine::Graphic& graphic)
	{
		return static_cast<ALLEGRO_BITMAP*>(graphic.handle().get());
	}

	struct Draw_Render_Visitor
	{
		struct clip_rect
		{
			int x, y, w, h;
		};

		std::stack<ALLEGRO_BITMAP*> old_target;
		std::stack<clip_rect> old_clipper;

		Draw_Render_Visitor()
		{ }

		void operator()(drawcmd::Push_Target& cmd)
		{
			auto bmp = cmd.bmp ? al_bmp(cmd.bmp) : nullptr;

			old_target.push(al_get_target_bitmap());

			if (bmp) al_set_target_bitmap(bmp);
		}

		void operator()(drawcmd::Pop_Target& cmd)
		{
			al_set_target_bitmap(old_target.top());
			old_target.pop();
		}

		void operator()(drawcmd::Push_Clipper& cmd)
		{
			clip_rect clip;
			al_get_clipping_rectangle(&clip.x, &clip.y, &clip.w, &clip.h);
			old_clipper.push(clip);
			al_set_clipping_rectangle(cmd.x, cmd.y, cmd.w, cmd.h);
		}

		void operator()(drawcmd::Pop_Clipper& cmd)
		{
			auto& clip = old_clipper.top();
			al_set_clipping_rectangle(clip.x, clip.y, clip.w, clip.h);
			old_clipper.pop();
		}

		void operator()(drawcmd::Clear& cmd)
		{
			al_clear_to_color(al_col(cmd.color));
		}

		void operator()(drawcmd::Clear_Depth& cmd)
		{
			al_clear_depth_buffer(cmd.depth);
		}

		void operator()(drawcmd::Blit& cmd)
		{
			if (!cmd.bmp)
				return;

			al_draw_bitmap(al_bmp(cmd.bmp), cmd.x, cmd.y, cmd.flags);
		}

		void operator()(drawcmd::Line& cmd)
		{
			al_draw_line(cmd.x, cmd.y, cmd.x2, cmd.y2, al_col(cmd.color), 0.0);
		}

		void operator()(drawcmd::Filled_Rect& cmd)
		{
			al_draw_filled_rectangle(cmd.x, cmd.y, cmd.x2, cmd.y2, al_col(cmd.color));
		}

		void operator()(drawcmd::Sync& cmd)
		{ }
	};
}

Engine_Allegro::Engine_Allegro()
	: m_accel(g_config.DrawAccel)
{ }

Engine_Allegro::~Engine_Allegro()
{ }

Engine::Graphic Engine_Allegro::create_texture(unsigned short width, unsigned short height)
{
	alsmart::unique_bitmap_flags flag_lock(m_accel ? ALLEGRO_VIDEO_BITMAP : ALLEGRO_MEMORY_BITMAP);

	auto bmp = alsmart::create_bitmap_shared(width, height);
	return Graphic(std::static_pointer_cast<void>(bmp), width, height);
}

Engine::Graphic Engine_Allegro::load_bmp(const char* bmp_start, std::size_t bmp_size)
{
	alsmart::unique_bitmap_flags flag_lock(m_accel ? ALLEGRO_VIDEO_BITMAP : ALLEGRO_MEMORY_BITMAP);

	void* mem = static_cast<void*>(const_cast<char*>(bmp_start));
	auto f = alsmart::open_memfile_unique(mem, bmp_size, "r");
	auto bmp = alsmart::load_bitmap_f_shared(f.get(), ".bmp");

	unsigned short width = al_get_bitmap_width(bmp.get());
	unsigned short height = al_get_bitmap_height(bmp.get());

	return Graphic(std::static_pointer_cast<void>(bmp), width, height);
}

void Engine_Allegro::render(Draw_Buffer& draw_buffer)
{
	//print_draw_commands(draw_buffer);

	Draw_Render_Visitor v;

	for (auto& cmd : draw_buffer.m_cmd_buffer)
		std::visit(v, cmd);

	al_flip_display();
}

void Engine_Allegro::render_to_target(Graphic& target, Draw_Buffer& draw_buffer)
{
	alsmart::unique_draw_target(al_bmp(target));

	render(draw_buffer);
}
