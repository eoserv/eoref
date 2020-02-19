#include "packet_processor.hpp"

#include "cio/cio.hpp" // debug

#include <utility>

namespace eo_protocol
{


void Packet_Processor::swap_multiples(char* buf, std::size_t n, eo_byte multi)
{
	auto swap_f = [&](std::size_t start, std::size_t length)
	{
		std::size_t end = start + length - 1;

		for (std::size_t i = 0; i < length / 2; ++i)
		{
			std::swap(buf[start + i], buf[end - i]);
		}
	};

	std::size_t sequence_length = 0;

	for (std::size_t i = 0; i < n; ++i)
	{
		eo_byte b = buf[i];

		if (b % multi == 0)
		{
			++sequence_length;
		}
		else
		{
			if (sequence_length > 1)
				swap_f(i - sequence_length, sequence_length);

			sequence_length = 0;
		}
	}

	if (sequence_length > 1)
		swap_f(n - sequence_length, sequence_length);
}

Packet_Processor::Packet_Processor()
{
	m_buf.resize(2048);
}

void Packet_Processor::decode(char* buf, std::size_t n)
{
	if (!ready())
		return;

	if (eo_byte(buf[0]) == 0xFF && eo_byte(buf[1]) == 0xFF)
		return;

	if (m_buf.size() < n)
		m_buf.resize(n);

	cio::err << "RecvE: ";

	for (std::size_t i = 0; i < n; ++i)
		cio::err << unsigned(eo_byte(buf[i])) << ' ';

	cio::err << cio::endl;

	std::size_t big_half = ((n + 1) / 2);
	std::size_t little_half = (n / 2);

	for (std::size_t i = 0; i < big_half; ++i)
		m_buf[i] = buf[i * 2];

	for (std::size_t i = 0; i < little_half; ++i)
		m_buf[n - 1 - i] = buf[(i * 2) + 1];

	for (std::size_t i = 0; i < n; ++i)
		buf[i] = eo_byte(m_buf[i]) ^ 0x80;

	for (std::size_t i = 0; i < n; ++i)
	{
		if (eo_byte(buf[i]) == 0)
			buf[i] = 128;
		else if (eo_byte(buf[i]) == 128)
			buf[i] = 0;
	}

	cio::err << "Recv : ";

	for (std::size_t i = 0; i < n; ++i)
		cio::err << unsigned(eo_byte(buf[i])) << ' ';

	cio::err << cio::endl;

	swap_multiples(buf, n, m_multi_d);
}

void Packet_Processor::encode(char* buf, std::size_t n)
{
	if (!ready())
		return;

	if (eo_byte(buf[0]) == 0xFF && eo_byte(buf[1]) == 0xFF)
		return;

	if (m_buf.size() < n)
		m_buf.resize(n);

	cio::err << "Send : ";

	for (std::size_t i = 0; i < n; ++i)
		cio::err << unsigned(eo_byte(buf[i])) << ' ';

	cio::err << cio::endl;

	swap_multiples(buf, n, m_multi_e);

	for (std::size_t i = 0; i < n; ++i)
	{
		if (eo_byte(buf[i]) == 0)
			buf[i] = 128;
		else if (eo_byte(buf[i]) == 128)
			buf[i] = 0;
	}

	std::size_t big_half = ((n + 1) / 2);
	std::size_t little_half = (n / 2);

	for (std::size_t i = 0; i < big_half; ++i)
		m_buf[i * 2] = buf[i];

	for (std::size_t i = 0; i < little_half; ++i)
		m_buf[(i * 2) + 1] = buf[n - 1 - i];

	for (std::size_t i = 0; i < n; ++i)
		buf[i] = eo_byte(m_buf[i]) ^ 0x80;

	cio::err << "SendE: ";

	for (std::size_t i = 0; i < n; ++i)
		cio::err << unsigned(eo_byte(buf[i])) << ' ';

	cio::err << cio::endl;
}

void Packet_Processor::set_multi(eo_byte d, eo_byte e)
{
	m_multi_d = d;
	m_multi_e = e;
}

bool Packet_Processor::ready() const
{
	return m_multi_d != 0;
}


}
