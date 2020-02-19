#ifndef EO_APP_HPP
#define EO_APP_HPP

#include "netclient.hpp"

#include "util/signal.hpp"

#include <memory>

// Only concerned with logical keys which are related to EO
// Extra keys are commented out
enum class AppKey
{
	Unmapped,

	//Num0, Num1, Num2, Num3, Num4,
	//Num5, Num6, Num7, Num8, Num9,

	//A, B, C, D, E, F, G, H, I, J, K, L, M,
	//N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

	//Tilde,
	//Minus,
	//Equals,
	//OpenBrace,
	//CloseBrace,
	//Semicolon,
	//Quote,
	//Backslash,
	//Comma,
	//Fullstop,
	//Slash,
	//Space,

	Shift,
	Ctrl,
	//Alt,
	//AltGr,

	Left,
	Right,
	Up,
	Down,

	F1, F2, F3, F4, F5, F6,
	F7, F8, F9, F10, F11, F12,

	Pad0, Pad1, Pad2, Pad3, Pad4,
	Pad5, Pad6, Pad7, Pad8, Pad9,

	//PadSlash,
	//PadAsterisk,
	//PadMinus,
	//PadPlus,
	PadDot,

	Enter,
	Escape,
	Backspace,
	Insert,
	Delete,
	Home,
	End,
	PgUp,
	PgDn
};

static constexpr AppKey AppKey_Max = AppKey::PgDn;

enum class AppMouseButton
{
	Unknown,

	Left,
	Middle,
	Right
};

// CP1252 "ANSI" codes
using AppChar = std::uint8_t;

class App
{
	private:
		struct impl_t;
		std::unique_ptr<impl_t> m_impl;

		bool m_open = false;

	public:
		util::signal<void(AppChar)> sig_key_char;
		util::signal<void(AppKey)> sig_key_down;
		util::signal<void(AppKey)> sig_key_up;

		util::signal<void(int, int)> sig_mouse_move;
		util::signal<void(AppMouseButton, int, int)> sig_mouse_down;
		util::signal<void(AppMouseButton, int, int)> sig_mouse_up;
		util::signal<void()> sig_mouse_enter;
		util::signal<void()> sig_mouse_leave;

		util::signal<void()> sig_window_focus;
		util::signal<void()> sig_window_unfocus;
		util::signal<void()> sig_window_expose;
		util::signal<void()> sig_window_close;

		// Triggered 8 times a second
		util::signal<void()> sig_tick;

		App();
		~App();

		void run();

		void close();

	friend struct impl_t;
};

#endif // EO_APP_HPP
