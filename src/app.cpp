#include "app.hpp"

#include "alsmart/alsmart.hpp"
#include "cio/cio.hpp"
#include "packet/eo_packets.hpp"
#include "trace.hpp"

#include <deque>
#include <functional>
#include <mutex>

#define TRACE_CTX "app"

static constexpr int TICK_RATE_FPS = 16;
static constexpr int DISPLAY_W = 640;
static constexpr int DISPLAY_H = 480;

// An arbitrary unused character code
static constexpr AppChar APPCHAR_UNMAPPED = 0x81;

static AppChar app_unicode_to_cp1251(int unichar)
{
	if ((unichar >= 0x00 && unichar <= 0x7F)
	 || (unichar >= 0xA0 && unichar <= 0xFF))
		return AppChar(unichar);

	// Many of the characters in the 80-9F range are inconsistent between fonts
	// Some of these may not render in all of EO's fonts
	switch (unichar)
	{
		case 0x20AC: return 0x80; // €
			// No mapping to 0x81
		case 0x201A: return 0x82; // ‚
		case 0x0192: return 0x83; // ƒ
		case 0x201E: return 0x84; // „
		case 0x2026: return 0x85; // …
		case 0x2020: return 0x86; // †
		case 0x2021: return 0x87; // ‡
		case 0x02C6: return 0x88; // ˆ
		case 0x2030: return 0x89; // ‰
		case 0x0160: return 0x8A; // Š
		case 0x2039: return 0x8B; // ‹
		case 0x0152: return 0x8C; // Œ
			// No mapping to 0x8D
		case 0x017D: return 0x8E; // Ž
			// No mapping to 0x8F
			// No mapping to 0x90
		case 0x2018: return 0x91; // ‘
		case 0x2019: return 0x92; // ’
		case 0x201C: return 0x93; // “
		case 0x201D: return 0x94; // ”
		case 0x2022: return 0x95; // •
		case 0x2013: return 0x96; // –
		case 0x2014: return 0x97; // —
		case 0x02DC: return 0x98; // ˜
		case 0x2122: return 0x99; // ™
		case 0x0161: return 0x9A; // š
		case 0x203A: return 0x9B; // ›
		case 0x0153: return 0x9C; // œ
			// No mapping to 0x9D
		case 0x017E: return 0x9E; // ž
		case 0x0178: return 0x9F; // Ÿ
	}

	return APPCHAR_UNMAPPED;
}

static AppKey app_keycode_to_appkey(int keycode)
{
	switch (keycode)
	{
		case ALLEGRO_KEY_LSHIFT:
		case ALLEGRO_KEY_RSHIFT:
			return AppKey::Shift;

		case ALLEGRO_KEY_LCTRL:
		case ALLEGRO_KEY_RCTRL:
			return AppKey::Ctrl;

		case ALLEGRO_KEY_LEFT:  return AppKey::Left;
		case ALLEGRO_KEY_RIGHT: return AppKey::Right;
		case ALLEGRO_KEY_UP:    return AppKey::Up;
		case ALLEGRO_KEY_DOWN:  return AppKey::Down;

		case ALLEGRO_KEY_F1:  return AppKey::F1;
		case ALLEGRO_KEY_F2:  return AppKey::F2;
		case ALLEGRO_KEY_F3:  return AppKey::F3;
		case ALLEGRO_KEY_F4:  return AppKey::F4;
		case ALLEGRO_KEY_F5:  return AppKey::F5;
		case ALLEGRO_KEY_F6:  return AppKey::F6;
		case ALLEGRO_KEY_F7:  return AppKey::F7;
		case ALLEGRO_KEY_F8:  return AppKey::F8;
		case ALLEGRO_KEY_F9:  return AppKey::F9;
		case ALLEGRO_KEY_F10: return AppKey::F10;
		case ALLEGRO_KEY_F11: return AppKey::F11;
		case ALLEGRO_KEY_F12: return AppKey::F12;

		case ALLEGRO_KEY_PAD_DELETE: return AppKey::PadDot;
		case ALLEGRO_KEY_PAD_0:      return AppKey::Pad0;
		case ALLEGRO_KEY_PAD_1:      return AppKey::Pad1;
		case ALLEGRO_KEY_PAD_2:      return AppKey::Pad2;
		case ALLEGRO_KEY_PAD_3:      return AppKey::Pad3;
		case ALLEGRO_KEY_PAD_4:      return AppKey::Pad4;
		case ALLEGRO_KEY_PAD_5:      return AppKey::Pad5;
		case ALLEGRO_KEY_PAD_6:      return AppKey::Pad6;
		case ALLEGRO_KEY_PAD_7:      return AppKey::Pad7;
		case ALLEGRO_KEY_PAD_8:      return AppKey::Pad8;
		case ALLEGRO_KEY_PAD_9:      return AppKey::Pad9;

		case ALLEGRO_KEY_ENTER:
		case ALLEGRO_KEY_PAD_ENTER:
			return AppKey::Enter;

		case ALLEGRO_KEY_ESCAPE:    return AppKey::Escape;
		case ALLEGRO_KEY_BACKSPACE: return AppKey::Backspace;
		case ALLEGRO_KEY_INSERT:    return AppKey::Insert;
		case ALLEGRO_KEY_DELETE:    return AppKey::Delete;
		case ALLEGRO_KEY_HOME:      return AppKey::Home;
		case ALLEGRO_KEY_END:       return AppKey::End;
		case ALLEGRO_KEY_PGUP:      return AppKey::PgUp;
		case ALLEGRO_KEY_PGDN:      return AppKey::PgDn;
	}

	return AppKey::Unmapped;
}

#ifdef linux
AppChar linux_fix_ctrl_keycode(ALLEGRO_EVENT& e)
{
	auto keycode = e.keyboard.keycode;
	auto unichar = e.keyboard.unichar;

/*
	From XkbToControl() in libX11:

    if ((c >= '@' && c < '\177') || c == ' ')
        c &= 0x1F;
    else if (c == '2')
        c = '\000';
    else if (c >= '3' && c <= '7')
        c -= ('3' - '\033');
    else if (c == '8')
        c = '\177';
    else if (c == '/')
        c = '_' & 0x1F;
*/

	bool capslock = e.keyboard.modifiers & ALLEGRO_KEYMOD_CAPSLOCK;
	bool shift = e.keyboard.modifiers & ALLEGRO_KEYMOD_SHIFT;

	AppChar lettershift = 0x20 * (capslock != shift);

#define PAIR(unichar, keycode) ((unsigned(unichar) << 16) | (unsigned(keycode) & 0xFFFF))

	switch (PAIR(unichar, keycode))
	{
		case PAIR(0x00, ALLEGRO_KEY_SPACE): return ' ';
		case PAIR(0x00, ALLEGRO_KEY_TILDE): return '`';

		case PAIR(0x00, ALLEGRO_KEY_2): return "2@"[shift];
		case PAIR(0x01, ALLEGRO_KEY_A): return 'a' ^ lettershift;
		case PAIR(0x02, ALLEGRO_KEY_B): return 'b' ^ lettershift;
		case PAIR(0x03, ALLEGRO_KEY_C): return 'c' ^ lettershift;
		case PAIR(0x04, ALLEGRO_KEY_D): return 'd' ^ lettershift;
		case PAIR(0x05, ALLEGRO_KEY_E): return 'e' ^ lettershift;
		case PAIR(0x06, ALLEGRO_KEY_F): return 'f' ^ lettershift;
		case PAIR(0x07, ALLEGRO_KEY_G): return 'g' ^ lettershift;
		case PAIR(0x08, ALLEGRO_KEY_H): return 'h' ^ lettershift;
		case PAIR(0x09, ALLEGRO_KEY_I): return 'i' ^ lettershift;
		case PAIR(0x0A, ALLEGRO_KEY_J): return 'j' ^ lettershift;
		case PAIR(0x0B, ALLEGRO_KEY_K): return 'k' ^ lettershift;
		case PAIR(0x0C, ALLEGRO_KEY_L): return 'l' ^ lettershift;
		case PAIR(0x0D, ALLEGRO_KEY_M): return 'm' ^ lettershift;
		case PAIR(0x0E, ALLEGRO_KEY_N): return 'n' ^ lettershift;
		case PAIR(0x0F, ALLEGRO_KEY_O): return 'o' ^ lettershift;
		case PAIR(0x10, ALLEGRO_KEY_P): return 'p' ^ lettershift;
		case PAIR(0x11, ALLEGRO_KEY_Q): return 'q' ^ lettershift;
		case PAIR(0x12, ALLEGRO_KEY_R): return 'r' ^ lettershift;
		case PAIR(0x13, ALLEGRO_KEY_S): return 's' ^ lettershift;
		case PAIR(0x14, ALLEGRO_KEY_T): return 't' ^ lettershift;
		case PAIR(0x15, ALLEGRO_KEY_U): return 'u' ^ lettershift;
		case PAIR(0x16, ALLEGRO_KEY_V): return 'v' ^ lettershift;
		case PAIR(0x17, ALLEGRO_KEY_W): return 'w' ^ lettershift;
		case PAIR(0x18, ALLEGRO_KEY_X): return 'x' ^ lettershift;
		case PAIR(0x19, ALLEGRO_KEY_Y): return 'y' ^ lettershift;
		case PAIR(0x1A, ALLEGRO_KEY_Z): return 'z' ^ lettershift;
		case PAIR(0x1B, ALLEGRO_KEY_3): return '3';
		case PAIR(0x1C, ALLEGRO_KEY_4): return '4';
		case PAIR(0x1D, ALLEGRO_KEY_5): return '5';
		case PAIR(0x1E, ALLEGRO_KEY_6): return "6^"[shift];
		case PAIR(0x1F, ALLEGRO_KEY_7): return '7';
		case PAIR(0x7F, ALLEGRO_KEY_8): return '8';

		case PAIR(0x1B, ALLEGRO_KEY_OPENBRACE):  return "[{"[shift];
		case PAIR(0x1C, ALLEGRO_KEY_BACKSLASH):  return "\\|"[shift];
		case PAIR(0x1D, ALLEGRO_KEY_CLOSEBRACE): return "]}"[shift];
		case PAIR(0x1E, ALLEGRO_KEY_TILDE):      return '~';
		case PAIR(0x1F, ALLEGRO_KEY_SLASH):      return '/';
		case PAIR(0x1F, ALLEGRO_KEY_MINUS):      return '_';
	}

#undef PAIR

	// No need to map it because this function is only called for ASCII-range
	return AppChar(unichar);
}
#endif // WIN32

struct App::impl_t
{
	App& app;

	alsmart::unique_display display;
	alsmart::unique_timer tick_timer;
	alsmart::unique_event_queue queue;

	bool kbd_state[ALLEGRO_KEY_MAX+1] = {};

	impl_t(App& app)
		: app(app)
	{
		// Disable resizable screen for now
		// TODO: full-hw and hw-composited modes should be resizable
		al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_GENERATE_EXPOSE_EVENTS);

		al_set_new_display_option(ALLEGRO_AUTO_CONVERT_BITMAPS, 0, ALLEGRO_REQUIRE);
		al_set_new_display_option(ALLEGRO_SINGLE_BUFFER, 1, ALLEGRO_SUGGEST);
		al_set_new_display_option(ALLEGRO_VSYNC, 0, ALLEGRO_SUGGEST);

		display = alsmart::create_display_unique(DISPLAY_W, DISPLAY_H);
		tick_timer = alsmart::create_timer_unique(1.0 / TICK_RATE_FPS);
		queue = alsmart::create_event_queue_unique();

		al_register_event_source(queue.get(), al_get_keyboard_event_source());
		al_register_event_source(queue.get(), al_get_mouse_event_source());
		al_register_event_source(queue.get(), al_get_display_event_source(display.get()));
		al_register_event_source(queue.get(), al_get_timer_event_source(tick_timer.get()));
	}

	bool check_key_x(int keycode)
	{
		auto check = [this](auto pair)
		{
			return !kbd_state[pair];
		};

		switch (keycode)
		{
			case ALLEGRO_KEY_LSHIFT: return check(ALLEGRO_KEY_RSHIFT);
			case ALLEGRO_KEY_RSHIFT: return check(ALLEGRO_KEY_LSHIFT);
			case ALLEGRO_KEY_LCTRL:  return check(ALLEGRO_KEY_RCTRL);
			case ALLEGRO_KEY_RCTRL:  return check(ALLEGRO_KEY_LCTRL);
		}

		return check(keycode);
	}

	bool check_key_down(int keycode)
	{
		if (keycode > ALLEGRO_KEY_MAX)
			return false;

		auto result = check_key_x(keycode);

		kbd_state[keycode] = true;

		return result;
	}

	bool check_key_up(int keycode)
	{
		if (keycode > ALLEGRO_KEY_MAX)
			return false;

		kbd_state[keycode] = false;

		return check_key_x(keycode);
	}

	void handle_key_char(ALLEGRO_EVENT& e, util::signal<void(AppChar)>& sig_key_char)
	{
		auto appkey = app_keycode_to_appkey(e.keyboard.keycode);
		auto appchar = app_unicode_to_cp1251(e.keyboard.unichar);

#ifdef linux
		// Hack to reverse ctrl-code mapping in X11
		if (appchar <= 0x1F || appchar == 0x7F)
			appchar = linux_fix_ctrl_keycode(e);
#endif

		if (appkey == AppKey::Unmapped && appchar != APPCHAR_UNMAPPED)
		{
			//trace_log("KeyChar: " << appchar << " (" << int(appchar) << ")");
			sig_key_char(appchar);
		}
	}

	void handle_key_down(ALLEGRO_EVENT& e, util::signal<void(AppKey)>& sig_key_down)
	{
		auto keycode = e.keyboard.keycode;

		if (check_key_down(keycode))
		{
			//trace_log("KeyDown:" << al_keycode_to_name(keycode));
			auto appkey = app_keycode_to_appkey(keycode);
			sig_key_down(appkey);
		}
		else
		{
			//trace_log("KeyDown (blocked): " << al_keycode_to_name(keycode));
		}
	}

	void handle_key_up(ALLEGRO_EVENT& e, util::signal<void(AppKey)>& sig_key_up)
	{
		auto keycode = e.keyboard.keycode;

		if (check_key_up(keycode))
		{
			//trace_log("KeyUp:" << al_keycode_to_name(keycode));
			auto appkey = app_keycode_to_appkey(keycode);
			sig_key_up(appkey);
		}
		else
		{
			//trace_log("KeyUp (blocked): " << al_keycode_to_name(keycode));
		}
	}
};

App::App()
	: m_impl(std::make_unique<impl_t>(*this))
{ }

App::~App()
{ }

void App::run()
{
	auto handle_allegro_event = [this](ALLEGRO_EVENT& e)
	{
		switch (e.type)
		{
			case ALLEGRO_EVENT_KEY_CHAR:
				m_impl->handle_key_char(e, sig_key_char);
				break;

			case ALLEGRO_EVENT_KEY_DOWN:
				m_impl->handle_key_down(e, sig_key_down);
				break;

			case ALLEGRO_EVENT_KEY_UP:
				m_impl->handle_key_up(e, sig_key_up);
				break;

			case ALLEGRO_EVENT_MOUSE_AXES:
			case ALLEGRO_EVENT_MOUSE_WARPED:
			{
				// TODO: Translate /dedupe coordinates for rescaling
				sig_mouse_move(e.mouse.x, e.mouse.y);
				break;
			}

			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			{
				// TODO: Translate /dedupe coordinates for rescaling
				if (e.mouse.button >= 1 && e.mouse.button <= 3)
					sig_mouse_down(AppMouseButton(e.mouse.button), e.mouse.x, e.mouse.y);
				break;
			}

			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			{
				// TODO: Translate /dedupe coordinates for rescaling
				if (e.mouse.button >= 1 && e.mouse.button <= 3)
					sig_mouse_up(AppMouseButton(e.mouse.button), e.mouse.x, e.mouse.y);
				break;
			}

			case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
				sig_mouse_enter();
				break;

			case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
				sig_mouse_leave();
				break;

			case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
				sig_window_focus();
				break;

			case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
				sig_window_unfocus();
				break;

			case ALLEGRO_EVENT_DISPLAY_EXPOSE:
			case ALLEGRO_EVENT_DISPLAY_FOUND:
				sig_window_expose();
				break;

			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				sig_window_close();
				break;

			case ALLEGRO_EVENT_TIMER:
				sig_tick();
				break;
		}
	};

	m_open = true;

	al_start_timer(m_impl->tick_timer.get());

	while (m_open)
	{
		ALLEGRO_EVENT e;

		al_wait_for_event(m_impl->queue.get(), &e);

		do
		{
			handle_allegro_event(e);
		} while(al_get_next_event(m_impl->queue.get(), &e));
	}

	m_impl->display.reset();
}

void App::close()
{
	// Triggers App::run() to close the display and return
	m_open = false;
}
