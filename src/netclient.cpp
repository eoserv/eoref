#include "netclient.hpp"

#include "data/eo_stream.hpp"
#include "packet/eo_packets.hpp"
#include "packet/packet_processor.hpp"

#include "trace.hpp"

#include <asio.hpp>

#include <array>
#include <deque>
#include <memory>
#include <optional>

namespace ip = asio::ip;
using tcp = asio::ip::tcp;

#define TRACE_CTX "netclient"

static void write_packet_hex(cio::stream& os, const char* data, std::size_t n)
{
	static constexpr const char hex[] = "0123456789ABCDEF";

	for (std::size_t i = 0; i < n; ++i)
	{
		eo_byte c = eo_byte(data[i]);
		cio::out << hex[(c & 0xF0) >> 8] << hex[c & 0x0F];
		cio::out << ' ';
	}
}

static cio::stream& operator<<(cio::stream& os, EO_Stream_Reader& reader)
{
	std::string_view str = reader.get();
	write_packet_hex(os, str.data(), str.size());
	return os;
}

static cio::stream& operator<<(cio::stream& os, EO_Stream_Builder& builder)
{
	std::string str = builder.get();
	write_packet_hex(os, str.data(), str.size());
	return os;
}

struct NetClient::impl_t
{
	// m_netclient and m_impl are valid for the lifetime of this class
	NetClient& m_netclient;

	asio::io_context m_io_ctx;
	tcp::socket m_client;

	state_t m_state = disconnected;

	std::array<char, 0xFFFF> m_client_read_buffer;
	std::size_t m_client_read_state = 0;
	Packet_Processor m_processor;
	unsigned m_seq_start = 0;
	unsigned m_seq = 0;

	unsigned next_seq()
	{
		unsigned result = (m_seq_start + m_seq) & 0xFFFFFFFFU;
		m_seq = (m_seq + 1) % 10;
		return result;
	}

	void set_state(state_t state)
	{
		if (m_state != state)
		{
			m_state = state;
			m_netclient.sig_state_change(state);
		}
	}

	void connect(std::string_view host, std::string_view port)
	{
		trace_log("Resolving " << host << ":" << port);

		set_state(connecting);

		tcp::resolver resolver(m_io_ctx);

		struct state_t
		{
			tcp::resolver::results_type::iterator it;
			tcp::resolver::results_type::iterator end;
		};

		auto&& state = std::make_shared<state_t>();

		resolver.async_resolve(host, port, tcp::resolver::numeric_service,
			[this, state](asio::error_code ec, tcp::resolver::results_type resolve_result)
			{
				if (ec)
				{
					trace_log("Host lookup failed");
					set_state(disconnected);
					return;
				}

				async_connect(m_client, resolve_result,
					[this, state](asio::error_code ec, const asio::ip::tcp::endpoint& endpoint)
					{
						if (ec)
						{
							trace_log("Could not connect");
							set_state(disconnected);
							return;
						}

						trace_log("Connected to " << endpoint.address().to_string());
						set_state(connected);
					}
				);
			}
		);
	}

	void disconnect()
	{
		if (m_state != disconnected)
		{
			trace_log("closing client");

			if (m_client.is_open())
				m_client.close();

			set_state(disconnected);
		}
	}

	void do_read()
	{
		if (m_client_read_state == 0)
		{
			asio::async_read(m_client,
				asio::buffer(&m_client_read_buffer[0], 2),
				[this](const asio::error_code& error, std::size_t bytes_transferred)
				{
					if (error)
					{
						trace_log("read error: " << error.message());
						disconnect();
						return;
					}

					unsigned a = eo_byte(m_client_read_buffer[0]);
					unsigned b = eo_byte(m_client_read_buffer[1]);

					unsigned length = eo_number_decode(a, b);

					m_client_read_state = std::size_t(length);
					m_io_ctx.post([this]() { do_read(); });
				}
			);
		}
		else
		{
			trace_log("attempting to read " << m_client_read_state << " bytes...");

			asio::async_read(m_client,
				asio::buffer(&m_client_read_buffer[0], m_client_read_state),
				[this](const asio::error_code& error, std::size_t bytes_transferred)
				{
					if (error)
					{
						trace_log("read error: " << error.message());
						disconnect();
						return;
					}

					m_processor.decode(&m_client_read_buffer[0], m_client_read_state);

					EO_Stream_Reader reader({
						m_client_read_buffer.data(),
						m_client_read_state
					});

					if (m_client_read_state < 2)
					{
						trace_log("dropping unknown packet (too short)");
						m_io_ctx.post([this]() { do_read(); });
						return;
					}

					trace_log("dump " << reader);

					auto packet = eo_protocol::unserialize(reader);

					if (!packet)
					{
						reader.seek(0);

						auto action = PacketAction(reader.get_byte());
						auto family = PacketFamily(reader.get_byte());

						trace_log("dropping unknown packet: " << name(family) << "_" << name(action));

						m_io_ctx.post([this]() { do_read(); });
						return;
					}

					auto packet_id = packet->vid();

					// Hijack pings to handle them automatically
					// And Init_Init to initialize packet processor
					if (packet_id == srv::Connection_Player::id)
					{
						handle_ping(packet->as<srv::Connection_Player>());
						return;
					}
					else if (packet_id == srv::Init_Init::id)
					{
						auto& init = packet->as<srv::Init_Init>();

						if (init.reply_code == eo_protocol::InitReply::OK)
						{
							m_processor.set_multi(init.u.ok.multi[0], init.u.ok.multi[1]);
							m_seq_start = init.u.ok.seq_start();
							set_state(ready);
						}
					}

					trace_log("recieved packet: " << name(packet_id.first) << "_" << name(packet_id.second));

					m_client_read_state = 0;
					m_io_ctx.post([this]() { do_read(); });

					m_netclient.sig_incoming_packet(*packet);
				}
			);
		}
	}

	void send_packet(PacketFamily family, PacketAction action,
							Client_Packet& packet)
	{
		std::size_t size = packet.byte_size();
		auto builder = std::make_shared<EO_Stream_Builder>(size + 4);

		// Insert the packet size directly in to the builder
		builder->add_short(size);
		builder->add_byte(eo_byte(action));
		builder->add_byte(eo_byte(family));

		trace_log("sending packet: " << name(family) << "_" << name(action));

		unsigned seq = next_seq();

		if (family != PacketFamily::Init || action != PacketAction::Init)
		{
			// TODO: builder.add_var(seq, 1, 2);
			if (seq > 254)
				builder->add_short(seq);
			else
				builder->add_char(seq);
		}

		packet.serialize(*builder);

		trace_log("dump " << *builder);

		std::string& packet_data = builder->get();

		// Packet_Builder is happy to let you mutate it in-place
		// Encode only the bytes of the packet after the length
		m_processor.encode(&packet_data[2], packet_data.size() - 2);

		// Important that the write handler captures builder by copy
		asio::async_write(m_client,
			asio::buffer(packet_data),
			[this, builder](const asio::error_code& error, std::size_t bytes_transferred)
			{
				if (error)
				{
					trace_log("write error: " << error.message());
					disconnect();
					return;
				}
			}
		);
	}

	void handle_ping(const srv::Connection_Player& packet)
	{
		m_seq_start = packet.seq_start();

		cli::Connection_Ping ping_reply;
		send_packet(ping_reply.family, ping_reply.action, ping_reply);
	}

	impl_t(NetClient& netclient)
		: m_netclient(netclient)
		, m_client(m_io_ctx)
	{ }
};

NetClient::NetClient()
	: m_impl(std::make_unique<impl_t>(*this))
{ }

NetClient::~NetClient()
{ }

void NetClient::connect(std::string_view host, std::string_view port)
{
	m_impl->connect(host, port);
}

void NetClient::disconnect()
{
	m_impl->disconnect();
}

void NetClient::send_packet(PacketFamily family, PacketAction action,
                            Client_Packet& packet)
{
	m_impl->send_packet(family, action, packet);
}
