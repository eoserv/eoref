#ifndef EO_PACKET_PACKET_BASE_HPP
#define EO_PACKET_PACKET_BASE_HPP

#include "data/eo_stream.hpp"
#include "data/eo_types.hpp"

#include <cstdint>
#include <cstdlib>
#include <string>

namespace eo_protocol
{
	enum class PacketFamily : eo_byte;
	enum class PacketAction : eo_byte;

	using PacketID = std::pair<PacketFamily, PacketAction>;

	struct Client_Packet
	{
		virtual ~Client_Packet();
		virtual std::size_t byte_size() const = 0;
		virtual void serialize(EO_Stream_Builder&) const = 0;
		virtual PacketID vid() const = 0;

		template <class T> T& as() { return *static_cast<T*>(this); }
		template <class T> const T& as() const { return *static_cast<const T*>(this); }
	};

	struct Server_Packet
	{
		virtual ~Server_Packet();
		virtual void unserialize(EO_Stream_Reader&) = 0;
		virtual PacketID vid() const = 0;

		template <class T> T& as() { return *static_cast<T*>(this); }
		template <class T> const T& as() const { return *static_cast<const T*>(this); }
	};
}

using eo_protocol::PacketFamily;
using eo_protocol::PacketAction;
using eo_protocol::PacketID;
using eo_protocol::Client_Packet;
using eo_protocol::Server_Packet;

#endif // EO2_EO_PACKET_HPP
