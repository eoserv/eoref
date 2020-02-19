#ifndef EO_ENGINE_HPP
#define EO_ENGINE_HPP

#include <cstdlib>
#include <memory>

class Draw_Buffer;

// Abstract interface to a rendering engine

// An engine has the following facets:
//   - Display and input management (currently part of App)
//   - Sprite loading
//   - Audio playback (todo)
//   - Drawing

// Engine functions should only be used from the main thread

class Engine
{
	public:
		class Graphic
		{
			private:
				std::shared_ptr<void> m_handle;
				unsigned short m_width;
				unsigned short m_height;

			public:
				Graphic() = default;

				Graphic(std::shared_ptr<void> handle, unsigned short width,
				        unsigned short height)
					: m_handle(handle)
					, m_width(width)
					, m_height(height)
				{ }

				std::shared_ptr<void> handle() { return m_handle; }

				int width() const { return m_width; }
				int height() const { return m_height; }

				operator bool() const { return m_handle != nullptr; }
		};

		virtual ~Engine();

		// Creates a blank graphic to be used with render_to_target
		virtual Graphic create_texture(unsigned short width, unsigned short height) = 0;

		// We have a dedicated load_bmp function to take advantage of Allegro's BMP loader
		virtual Graphic load_bmp(const char* bmp_start, std::size_t bmp_size) = 0;

		// renders to the "display", as determined by the App class
		virtual void render(Draw_Buffer&) = 0;

		// renders to a target Graphic (should be created by create_texture)
		virtual void render_to_target(Graphic&, Draw_Buffer&) = 0;
};

// Global engine object
extern Engine* g_engine;

// Initialzie g_engine based on the current configuration
void eo_init_engine();

// Tear down the engine and clean up
void eo_destroy_engine();

#endif // EO_ENGINE_HPP
