#ifndef EO_PACKETS_HPP
#define EO_PACKETS_HPP

#include "eo_protocol.hpp"

#include "eo_protocol/client.hpp"
#include "eo_protocol/server.hpp"

#include <memory>

namespace cli = eo_protocol::client;
namespace srv = eo_protocol::server;

namespace eo_protocol
{


std::shared_ptr<Server_Packet> unserialize(EO_Stream_Reader& reader);

constexpr unsigned packet_id_hash(PacketID id)
{
	unsigned f = eo_byte(id.first);
	unsigned a = eo_byte(id.second);

	return (f << 8) | a;
}

template <class T, class U>
struct copy_ref_type_impl;

template <class T, class U>
struct copy_ref_type_impl<T&, U>
	{ using type = U&; };

template <class T, class U>
struct copy_ref_type_impl<const T&, U>
	{ using type = const U&; };

template <class T, class U>
struct copy_ref_type_impl<T&&, U>
	{ using type = U&&; };

template <class T, class U>
	using copy_ref_type = typename copy_ref_type_impl<T, U>::type;

template <class Callable, class PacketT>
auto client_visit_impl(Callable&& f, PacketT&& base)
	-> decltype(f(base))
{
	auto id = packet_id_hash(base.vid());

#define case_packet(type) \
	case packet_id_hash(client::type::id):\
	return f(static_cast<copy_ref_type<decltype(base), client::type>>(base));

	switch (id)
	{
#include "eo_protocol/client_packets.tpp"
	}
#undef case_packet
}

template <class Callable, class PacketT>
auto server_visit_impl(Callable&& f, PacketT&& base)
	-> decltype(f(base))
{
	auto id = packet_id_hash(base.vid());

#define case_packet(type) \
	case packet_id_hash(server::type::id): \
	return f(static_cast<copy_ref_type<decltype(base), server::type>>(base));

	switch (id)
	{
#include "eo_protocol/server_packets.tpp"
	}
#undef case_packet
}

template <class Callable>
auto visit(Callable&& f, Client_Packet& base)
{
	return client_visit_impl<Callable&&, Client_Packet&>(
		std::forward<Callable>(f), base);
}

template <class Callable>
auto visit(Callable&& f, const Client_Packet& base)
{
	return client_visit_impl<Callable&&, const Client_Packet&>(
		std::forward<Callable>(f), base);
}

template <class Callable>
auto visit(Callable&& f, Client_Packet&& base)
{
	return client_visit_impl<Callable&&, Client_Packet&&>
		(std::forward<Callable>(f), base);
}

template <class Callable>
auto visit(Callable&& f, Server_Packet& base)
{
	return server_visit_impl<Callable&&, Server_Packet&>
		(std::forward<Callable>(f), base);
}

template <class Callable>
auto visit(Callable&& f, const Server_Packet& base)
{
	return server_visit_impl<Callable&&, const Server_Packet&>
		(std::forward<Callable>(f), base);
}

template <class Callable>
auto visit(Callable&& f, Server_Packet&& base)
{
	return server_visit_impl<Callable&&, Server_Packet&&>(f, base);
}

const char* name(PacketFamily family);
const char* name(PacketAction action);


}

using namespace eo_protocol;

#endif // EO_PACKETS_HPP
