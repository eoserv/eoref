#ifndef EO_GAME_HPP
#define EO_GAME_HPP

#include "packet/eo_packets.hpp"
#include "util/signal.hpp"

#include "app.hpp"
#include "netclient.hpp"

#include <memory>

class Game
{
	private:
		NetClient m_netclient;

		App m_app;
		unsigned m_tick = 0;

		void handle_connect();
		void handle_disconnect();
		void handle_packet(Server_Packet& packet);

		void handle_key_char(AppChar c);
		void handle_key_down(AppKey key);
		void handle_key_up(AppKey key);

		void handle_mouse_move(int x, int y);
		void handle_mouse_down(AppMouseButton button, int x, int y);
		void handle_mouse_up(AppMouseButton button, int x, int y);
		void handle_mouse_enter();
		void handle_mouse_leave();

		void handle_window_focus();
		void handle_window_unfocus();
		void handle_window_expose();
		void handle_window_close();

		void handle_tick();

		void draw();

	public:
		Game();
		~Game();

		void run();

		void send_packet(Client_Packet& packet);
};

#endif // EO_GAME_HPP
