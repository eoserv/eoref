#ifndef PRINTER_HPP
#define PRINTER_HPP

#include "ast.hpp"

#include <iosfwd>
#include <string>
#include <utility>

class Printer
{
	private:
		ProtocolFile& m_proto;

		std::string make_byte_size_expression(DataBlockEntries& dbe) const;

	public:
		enum TypeType_t
		{
			type_break,
			type_builtin,
			type_enum
		};

		Printer(ProtocolFile& proto);

		TypeType_t type_type(const std::string& name) const;
		const EnumBlock& get_enum(const std::string& name) const;
		std::string get_enum_type(const DataBlockEntries& dbe, const std::string& field_name) const;

		std::size_t size_of(const std::string&) const;
		std::string add_fn(const std::string&) const;
		std::string get_fn(const std::string&) const;
		std::string enum_base_type(const std::string&) const;

		void print_data_block(std::ostream&, const DataBlockEntries&, int depth = 0) const;
		void print_serialize_code(std::ostream&, const DataBlockEntries&, int depth = 0, const std::string& prefix = {}) const;
		void print_unserialize_code(std::ostream&, const DataBlockEntries&, int depth = 0, const std::string& prefix = {}) const;

		void print_enum(std::ostream&, const std::pair<std::string, EnumBlock::ptr>&, int depth = 0) const;
		void print_struct(std::ostream&, const std::pair<std::string, StructBlock::ptr>&, int depth = 0) const;
		void print_struct_impl(std::ostream&, const std::pair<std::string, StructBlock::ptr>&, int depth = 0) const;
		void print_client_def(std::ostream&, const std::pair<std::string, PacketBlock::ptr>&, int depth = 0) const;
		void print_server_def(std::ostream&, const std::pair<std::string, PacketBlock::ptr>&, int depth = 0) const;
		void print_client_impl(std::ostream&, const std::pair<std::string, PacketBlock::ptr>&, int depth = 0) const;
		void print_server_impl(std::ostream&, const std::pair<std::string, PacketBlock::ptr>&, int depth = 0) const;

		void print_enums(std::ostream&, int depth = 0) const;
		void print_structs(std::ostream&, int depth = 0) const;
		void print_struct_impls(std::ostream&, int depth = 0) const;
		void print_client_packet_defs(std::ostream&, int depth = 0) const;
		void print_server_packet_defs(std::ostream&, int depth = 0) const;
		void print_client_packet_variant(std::ostream&, int depth = 0) const;
		void print_server_packet_variant(std::ostream&, int depth = 0) const;
		//void print_client_packet_variant(std::ostream&, int depth = 0) const;
		void print_client_packet_cases(std::ostream&, int depth = 0) const;
		void print_server_packet_cases(std::ostream&, int depth = 0) const;
		void print_client_packet_impls(std::ostream&, int depth = 0) const;
		void print_server_packet_impls(std::ostream&, int depth = 0) const;
};

#endif // PRINTER_HPP
