#ifndef EO_STREAM_HPP
#define EO_STREAM_HPP

#include "eo_types.hpp"

#include <cstdlib>
#include <string>
#include <string_view>

class EO_Stream_Reader
{
	private:
		std::string_view m_data;
		std::size_t m_pos;

	public:
		EO_Stream_Reader(std::string_view data);

		std::size_t length() const;
		std::size_t remaining() const;

		eo_byte get_byte();
		eo_char get_char();
		eo_short get_short();
		eo_int get_three();
		eo_int get_int();

		std::string_view get_fixed_string(std::size_t length);
		std::string_view get_break_string();
		std::string_view get_prefix_string();
		std::string_view get_end_string();

		bool unbroken() const;

		// For debugging purposes
		std::string_view get() const { return m_data; }

		std::size_t tell() const { return m_pos; }
		void seek(std::size_t offset);
		void skip(long offset);
		void seek_reverse(std::size_t offset);
};

class EO_Stream_Builder
{
	private:
		std::string m_data;
		std::size_t m_pos;

	public:
		EO_Stream_Builder(std::size_t size_guess = 0);

		std::size_t length() const;

		void add_byte(eo_byte n);
		void add_char(eo_char n);
		void add_short(eo_short n);
		void add_three(eo_int n);
		void add_int(eo_int n);

		void add_string(const std::string& str);
		void add_break_string(const std::string& str);
		void add_prefix_string(const std::string& str);

		std::string& get() { return m_data; }
		const std::string& get() const { return m_data; }

		std::size_t tell() const { return m_pos; }
		void seek(std::size_t offset);
		void skip(long offset);
		void seek_reverse(std::size_t offset);
};

#endif // EO_STREAM_HPP
