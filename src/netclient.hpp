#ifndef EO_NETCLIENT_HPP
#define EO_NETCLIENT_HPP

#include "packet/eo_packets.hpp"

#include "util/signal.hpp"

#include <memory>
#include <string_view>

class NetClient
{
	public:
		enum state_t
		{
			disconnected,
			connecting,
			connected,
			ready
		};

		util::signal<void(state_t)> sig_state_change;
		util::signal<void(Server_Packet&)> sig_incoming_packet;

	private:
		class impl_t;
		std::unique_ptr<impl_t> m_impl;

	public:
		NetClient();
		~NetClient();

		// no copy/move/assign
		NetClient(const NetClient&) = delete;
		const NetClient& operator=(const NetClient&) = delete;

		void connect(std::string_view host, std::string_view port);
		void disconnect();

		void send_packet(PacketFamily family, PacketAction action,
		                 Client_Packet& packet);

		template <class T>
		void send_packet(T& packet)
		{
			send_packet(T::family, T::action, packet);
		}
};

#endif // EO_NETCLIENT_HPP
