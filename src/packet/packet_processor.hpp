#ifndef EO_PACKET_PACKET_PROCESSOR_HPP
#define EO_PACKET_PACKET_PROCESSOR_HPP

#include "packet_base.hpp"

#include <vector>

namespace eo_protocol
{
	class Packet_Processor
	{
		private:
			std::vector<char> m_buf;
			eo_byte m_multi_d = 0;
			eo_byte m_multi_e = 0;

			void swap_multiples(char* buf, size_t n, eo_byte multi);

		public:
			Packet_Processor();

			void decode(char* buf, size_t n);
			void encode(char* buf, size_t n);

			void set_multi(eo_byte d, eo_byte e);

			bool ready() const;
	};
}

using eo_protocol::Packet_Processor;

#endif // EO_PACKET_PACKET_PROCESSOR_HPP
