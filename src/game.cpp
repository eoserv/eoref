#include "game.hpp"

#include "gfx/draw_buffer.hpp"

#include "trace.hpp"

#define TRACE_CTX "game"

void Game::handle_connect()
{
	trace_log("Connected");
}

void Game::handle_disconnect()
{
	trace_log("Disconnected");
}

void Game::handle_packet(Server_Packet& packet)
{
}

void Game::handle_key_char(AppChar c)
{
	//trace_log("Char " << c << "(" << int(c) << ")");
}

void Game::handle_key_down(AppKey key)
{
	//trace_log("KeyDown " << int(key));
}

void Game::handle_key_up(AppKey key)
{
	//trace_log("KeyUp " << int(key));
}

void Game::handle_mouse_move(int x, int y)
{
}

void Game::handle_mouse_down(AppMouseButton button, int x, int y)
{
}

void Game::handle_mouse_up(AppMouseButton button, int x, int y)
{
}

void Game::handle_window_close()
{
	m_app.close();
}

void Game::handle_tick()
{
	draw();
}

void Game::draw()
{
	Draw_Buffer drawbuf;
	//current_scene.draw(drawbuf);
	g_engine->render(drawbuf);
}

Game::Game()
{
	auto bind_this = [this](auto fptr)
	{
		return [this, fptr](auto&&... args)
		{
			return (this->*fptr)(args...);
		};
	};

	auto connect_this = [bind_this](auto&& sig, auto fptr)
	{
		sig.connect(bind_this(fptr));
	};

	m_netclient.sig_state_change.connect([this](NetClient::state_t state)
	{
		if (state == NetClient::connected)
			handle_connect();
		else if (state == NetClient::disconnected)
			handle_disconnect();
	});

	connect_this(m_netclient.sig_incoming_packet, &Game::handle_packet);

	connect_this(m_app.sig_key_char, &Game::handle_key_char);
	connect_this(m_app.sig_key_up, &Game::handle_key_up);
	connect_this(m_app.sig_key_down, &Game::handle_key_down);

	connect_this(m_app.sig_mouse_move, &Game::handle_mouse_move);
	connect_this(m_app.sig_mouse_down, &Game::handle_mouse_down);
	connect_this(m_app.sig_mouse_up, &Game::handle_mouse_up);

	connect_this(m_app.sig_window_close, &Game::handle_window_close);

	connect_this(m_app.sig_tick, &Game::handle_tick);
}

Game::~Game()
{ }

void Game::run()
{
	m_app.run();
}

void Game::send_packet(Client_Packet& packet)
{
}
