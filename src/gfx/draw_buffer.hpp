#ifndef EO_DRAW_BUFFER_HPP
#define EO_DRAW_BUFFER_HPP

#include "gfx/color.hpp"
//#include "gfx/gfx_pool.hpp"

// TODO: Engine::Graphic should be replaced with GFX_Pool's ref-type
#include "engine.hpp"

#include <deque>
#include <type_traits>
#include <variant>

// changing this to float may give performance benefit on Allegro
using drawcoord_t = unsigned short;

namespace drawcmd
{
	struct Push_Target { Engine::Graphic bmp; };
	struct Pop_Target { };
	struct Push_Clipper { drawcoord_t x, y, w, h; };
	struct Pop_Clipper { };

	struct Clear { Color color; };
	struct Clear_Depth { float depth; };

	struct Line { drawcoord_t x, y, x2, y2; Color color; };
	struct Filled_Rect { drawcoord_t x, y, x2, y2; Color color; };

	// TODO: Define some flags
	struct Blit { Engine::Graphic bmp; drawcoord_t x, y, z; int flags; };

	struct Sync { };
}

using Draw_Command = std::variant<
	drawcmd::Push_Target,
	drawcmd::Pop_Target,
	drawcmd::Push_Clipper,
	drawcmd::Pop_Clipper,
	drawcmd::Clear,
	drawcmd::Clear_Depth,
	drawcmd::Blit,
	drawcmd::Line,
	drawcmd::Filled_Rect,
	drawcmd::Sync
>;

class Draw_Buffer
{
	public:
		std::deque<Draw_Command> m_cmd_buffer;
		bool m_depth;

		Draw_Buffer(bool depth = true);
		~Draw_Buffer();

		void add(const Draw_Command& cmd);

		void push_target(const Engine::Graphic& bmp)
		{ add(drawcmd::Push_Target{ bmp }); }

		void pop_target()
		{ add(drawcmd::Pop_Target{}); }

		void push_clipper(drawcoord_t x, drawcoord_t y, drawcoord_t w, drawcoord_t h)
		{ add(drawcmd::Push_Clipper{ x, y, w, h }); }

		void pop_clipper()
		{ add(drawcmd::Pop_Clipper{}); }

		void clear(const Color& color)
		{ add(drawcmd::Clear{ color }); }

		void clear_depth(float depth)
		{ add(drawcmd::Clear_Depth{ depth }); }

		void draw(const Engine::Graphic& bmp, drawcoord_t x, drawcoord_t y, drawcoord_t z, int flags)
		{ add(drawcmd::Blit{ bmp, x, y, z, flags }); }

		void line(drawcoord_t x, drawcoord_t y, drawcoord_t x2, drawcoord_t y2,
		          const Color& color)
		{ add(drawcmd::Line{ x, y, x2, y2, color }); }

		void filled_rect(drawcoord_t x, drawcoord_t y, drawcoord_t x2, drawcoord_t y2,
		                 const Color& color)
		{ add(drawcmd::Filled_Rect{ x, y, x2, y2, color }); }

		void sync()
		{ add(drawcmd::Sync{}); }
};

class Draw_Unique_Target
{
	private:
		Draw_Buffer& m_buffer;

	public:
		Draw_Unique_Target(Draw_Buffer& buffer, Engine::Graphic target)
			: m_buffer(buffer)
		{
			m_buffer.push_target(target);
		}

		~Draw_Unique_Target()
		{
			m_buffer.pop_target();
		}
};

class Draw_Unique_Clipper
{
	private:
		Draw_Buffer& m_buffer;

	public:
		Draw_Unique_Clipper(Draw_Buffer& buffer, int x, int y, int x2, int y2)
			: m_buffer(buffer)
		{
			m_buffer.push_clipper(x, y, x2, y2);
		}

		~Draw_Unique_Clipper()
		{
			m_buffer.pop_clipper();
		}
};

#endif // EO_DRAW_BUFFER_HPP
