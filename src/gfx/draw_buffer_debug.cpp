#include "draw_buffer_debug.hpp"

#include "cio/cio.hpp"

#include <allegro5/allegro_primitives.h>

#include <cstdint>
#include <deque>
#include <map>
#include <stack>
#include <string>
#include <variant>

struct Draw_Render_Dump_Visitor
{
	std::map<std::uintptr_t, int> bmp_id_table;
	std::size_t last_cmd = -1U;
	int repeats = 0;

	const int max_repeats;

	Draw_Render_Dump_Visitor(int max_repeats)
		: max_repeats(max_repeats)
	{ }

	template <class T>
	bool check_repeats(T& cmd)
	{
		auto&& cmd_index = Draw_Command(cmd).index();

		if (cmd_index != last_cmd)
		{
			repeats = 0;
			last_cmd = cmd_index;
			return true;
		}

		++repeats;

		if (repeats < 3)
		{
			return true;
		}
		else if (repeats == 3)
		{
			cio::out << "...\n";
			return false;
		}
		else
		{
			return false;
		}
	}

	void operator()(drawcmd::Push_Target& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Push_Target, image:" << std::uintptr_t(cmd.bmp.handle().get()) << "]\n";
	}

	void operator()(drawcmd::Pop_Target& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Pop_Target]\n";
	}

	void operator()(drawcmd::Push_Clipper& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Push_Clipper, x:" << cmd.x << ", y:" << cmd.y <<
		            ", w:" << cmd.w << ", h:" << cmd.h << "]\n";
	}

	void operator()(drawcmd::Pop_Clipper& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Pop_Clipper]\n";
	}

	void operator()(drawcmd::Clear& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Clear, color:(" << cmd.color.r() << ',' << cmd.color.g()
		         << ',' << cmd.color.b() << ',' << cmd.color.a() << ")]\n";
	}

	void operator()(drawcmd::Clear_Depth& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Clear_Depth, depth:" << cmd.depth << "]\n";
	}

	void operator()(drawcmd::Line& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Line, x:" << cmd.x << ", y:" << cmd.y << ", x2:"
		         << cmd.x2 << ", y2:" << cmd.y2 << ", color:("
		         << cmd.color.r() << ',' << cmd.color.g() << ',' << cmd.color.b()
		         << ',' << cmd.color.a() << "]\n";
	}

	void operator()(drawcmd::Filled_Rect& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Filled_Rect, x:" << cmd.x << ", y:" << cmd.y << ", x2:"
		         << cmd.x2 << ", y2:" << cmd.y2 << ", color:("
		         << cmd.color.r() << ',' << cmd.color.g() << ',' << cmd.color.b()
		         << ',' << cmd.color.a() << "]\n";
	}

	void operator()(drawcmd::Blit& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Blit, image:" << std::uintptr_t(cmd.bmp.handle().get()) << ", x:"
		         << cmd.x << ", y:" << cmd.y << ", z:" << cmd.z << ", flags:"
		         << cmd.flags << "]\n";
	}

	void operator()(drawcmd::Sync& cmd)
	{
		if (check_repeats(cmd))
		cio::out << "[Sync]\n";
	}
};

void print_draw_commands(Draw_Buffer& draw_buf, int max_repeat)
{
	Draw_Render_Dump_Visitor visitor(max_repeat);

	for (auto&& cmd : draw_buf.m_cmd_buffer)
		std::visit(visitor, cmd);
}
