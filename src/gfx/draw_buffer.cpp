#include "draw_buffer.hpp"

Draw_Buffer::Draw_Buffer(bool depth)
	: m_depth(depth)
{ }

Draw_Buffer::~Draw_Buffer()
{ }

void Draw_Buffer::add(const Draw_Command& cmd)
{
	m_cmd_buffer.push_back(cmd);
}
