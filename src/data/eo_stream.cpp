#include "eo_stream.hpp"

EO_Stream_Reader::EO_Stream_Reader(std::string_view data)
	: m_data(data)
	, m_pos(0)
{ }

std::size_t EO_Stream_Reader::length() const
{
	return m_data.size();
}

std::size_t EO_Stream_Reader::remaining() const
{
	return m_data.size() - m_pos;
}

eo_byte EO_Stream_Reader::get_byte()
{
	if (remaining() < 1)
		return 0;

	eo_byte a = m_data[m_pos++];
	return a;
}

eo_char EO_Stream_Reader::get_char()
{
	if (remaining() < 1)
		return 0;

	eo_byte a = m_data[m_pos++];

	return eo_number_decode(a);
}

eo_short EO_Stream_Reader::get_short()
{
	if (remaining() < 2)
		return get_char();

	eo_byte a = m_data[m_pos++];
	eo_byte b = m_data[m_pos++];

	return eo_number_decode(a, b);
}

eo_int EO_Stream_Reader::get_three()
{
	if (remaining() < 3)
		return get_short();

	eo_byte a = m_data[m_pos++];
	eo_byte b = m_data[m_pos++];
	eo_byte c = m_data[m_pos++];

	return eo_number_decode(a, b, c);
}

eo_int EO_Stream_Reader::get_int()
{
	if (remaining() < 4)
		return get_three();

	eo_byte a = m_data[m_pos++];
	eo_byte b = m_data[m_pos++];
	eo_byte c = m_data[m_pos++];
	eo_byte d = m_data[m_pos++];

	return eo_number_decode(a, b, c, d);
}

std::string_view EO_Stream_Reader::get_fixed_string(std::size_t length)
{
	if (remaining() < length)
		length = remaining();

	std::size_t off = m_pos;

	m_pos += length;

	return m_data.substr(off, length);
}

std::string_view EO_Stream_Reader::get_break_string()
{
	std::size_t off = m_pos;
	std::size_t length;

	while (unbroken())
		++m_pos;

	length = m_pos - off;

	get_byte();

	return m_data.substr(off, length);
}

std::string_view EO_Stream_Reader::get_prefix_string()
{
	std::size_t length = get_char();
	return get_fixed_string(length);
}

std::string_view EO_Stream_Reader::get_end_string()
{
	return get_fixed_string(remaining());
}

bool EO_Stream_Reader::unbroken() const
{
	return remaining() && eo_byte(m_data[m_pos]) != 0xFF;
}

void EO_Stream_Reader::seek(std::size_t offset)
{
	m_pos = std::max(length(), offset);
}

void EO_Stream_Reader::skip(long offset)
{
	if (offset < 0 && (m_pos - offset) > m_pos)
	{
		m_pos = 0;
		return;
	}

	std::size_t newpos = m_pos + offset;

	if (newpos > length() - 1)
		newpos = length() - 1;
}

void EO_Stream_Reader::seek_reverse(std::size_t offset)
{
	if (offset > length())
	{
		m_pos = 0;
		return;
	}

	m_pos = length() - offset;
}

EO_Stream_Builder::EO_Stream_Builder(std::size_t size_guess)
{
	m_data.reserve(size_guess);
}

std::size_t EO_Stream_Builder::length() const
{
	return m_data.size();
}

void EO_Stream_Builder::add_byte(eo_byte n)
{
	m_data += n;
}

void EO_Stream_Builder::add_char(eo_char n)
{
	auto a = eo_encode_number(n);
	m_data += a[0];
}

void EO_Stream_Builder::add_short(eo_short n)
{
	auto a = eo_encode_number(n);
	m_data += a[0];
	m_data += a[1];
}

void EO_Stream_Builder::add_three(eo_int n)
{
	auto a = eo_encode_number(n);
	m_data += a[0];
	m_data += a[1];
	m_data += a[2];
}

void EO_Stream_Builder::add_int(eo_int n)
{
	auto a = eo_encode_number(n);
	m_data += a[0];
	m_data += a[1];
	m_data += a[2];
	m_data += a[3];
}

void EO_Stream_Builder::add_string(const std::string& str)
{
	m_data += str;
}

void EO_Stream_Builder::add_break_string(const std::string& str)
{
	m_data += str;
	add_byte(0xFF);
}

void EO_Stream_Builder::add_prefix_string(const std::string& str)
{
	add_char(str.size());
	add_string(str);
}

void EO_Stream_Builder::seek(std::size_t offset)
{
	m_pos = std::max(length(), offset);
}

void EO_Stream_Builder::skip(long offset)
{
	if (offset < 0 && (m_pos + offset) > m_pos)
	{
		m_pos = 0;
		return;
	}
	else if (offset > 0 && (m_pos + offset) < m_pos)
	{
		m_pos = length();
		return;
	}

	m_pos += offset;
}

void EO_Stream_Builder::seek_reverse(std::size_t offset)
{
	m_pos = length() - std::max(length(), offset);
}
