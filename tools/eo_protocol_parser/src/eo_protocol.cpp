#include "eo_protocol.hpp"

#include <iostream>

namespace eo_protocol
{


/*
void write_client_packet_struct(std::ostream& os, const ProtocolFile& ast,
                                const std::string& packet_name,
                                const PacketBlock& packet_block)
{

}

static void write_client_packet_byte_size(std::ostream& os, const ProtocolFile& ast,
                                          const std::string& packet_name,
                                          const PacketBlock& packet_block)
{
	int constant_part = 0;

	os << "std::size_t " << packet_name << "::byte_size() const\n{\n";
	os << "\treturn " << constant_part;

	for (const PacketBlockEntry& e : packet_block.dbe.entries)
	{

	}

	os << "\n}\n";
}

void write_client_packet_impl(std::ostream& os, const ProtocolFile& ast,
                              const std::string& packet_name,
                              const PacketBlock& packet_block)
{

}

void write_server_packet_struct(std::ostream& os, const ProtocolFile& ast,
                                const std::string& packet_name,
                                const PacketBlock& packet_block)
{

}

void write_server_packet_impl(std::ostream& os, const ProtocolFile& ast,
                              const std::string& packet_name,
                              const PacketBlock& packet_block)
{

}
*/


void DataField::from_ast(const EO_Protocol& proto, const ::DataField& ast)
{
	std::string base_type;

	if (ast.type_class)
	{
		auto enum_type_it = proto.enums.find(ast.type);

		if (enum_type_it == proto.enums.end())
			throw std::runtime_error("Unknown enum: " + ast.type);

		base_type = ast.type_class.value();
	}
	else
	{
		base_type = ast.type;

		     if (base_type == "byte")       ;
		else if (base_type == "char")       ;
		else if (base_type == "short")      ;
		else if (base_type == "three")      ;
		else if (base_type == "int")        ;
		else if (base_type == "string")     ;
		else if (base_type == "string_raw") ;
		else
		{
			auto it = proto.enums.find(base_type);

			if (it == proto.enums.end())
				throw std::runtime_error("Unknown type: " + ast.type);

			base_type = it->second.base_type;
		}
	}

	     if (base_type == "byte")       type = TYPE_BYTE;
	else if (base_type == "char")       type = TYPE_CHAR;
	else if (base_type == "short")      type = TYPE_SHORT;
	else if (base_type == "three")      type = TYPE_THREE;
	else if (base_type == "int")        type = TYPE_INT;
	else if (base_type == "string")     type = TYPE_STRING;
	else if (base_type == "string_raw") type = TYPE_STRING_RAW;
	else throw std::runtime_error("Internal error: Unknown type: " + ast.type);

	name = ast.name;

	if (ast.initializer)
		initializer = " = " + std::to_string(ast.initializer.value());

	if (ast.dynamic_size)
		size = ast.dynamic_size.value();
	else if (ast.static_size)
		size = ast.static_size.value();
}

void DataField::from_ast(const EO_Protocol& proto, const ::StructField& ast)
{
	type = TYPE_STRUCT;
	struct_type = ast.type;
	name = ast.name.value();

	if (ast.dynamic_size)
		size = ast.dynamic_size.value();
	else if (ast.static_size)
		size = ast.static_size.value();
}

void EO_Protocol::from_ast(const ::ProtocolFile& ast)
{
	for (auto& ep : ast.enums)
	{
		auto& enum_name = ep.first;
		auto& ast_e = ep.second;

		Enum e;
		e.base_type = ast_e->type_class.value_or("char");
		e.comment = ast_e->comment;
		e.values = ast_e->entries;

		enums.insert({enum_name, e});
	}

	auto pending_structs = ast.structs;
	std::deque<std::string> completed_structs;

	while (!pending_structs.empty())
	{
		for (auto& ast_struct_it : pending_structs)
		{
			auto& struct_name = ast_struct_it.first;
			auto& ast_struct = ast_struct_it.second;

			Struct s;
			s.comment = ast_struct->comment;

			for (auto ast_data_entry : ast_struct->dbe.entries)
			{
				switch (ast_data_entry.index()) // Lazy, stupid way of using a variant !
				{
					case 0: // DataField
					{
						const auto& ast_entry = std::get<std::shared_ptr<::DataField>>(ast_data_entry);

						if (ast_entry->type == "break")
						{
							s.entries.emplace_back(DataFieldBreak());
						}
						else
						{
							DataField df;
							df.from_ast(*this, *ast_entry);
							s.entries.push_back(df);
						}
					}
					break;

					case 1: // StructField
					{
						const auto& ast_entry = std::get<std::shared_ptr<::StructField>>(ast_data_entry);

						std::string entry_type = ast_entry->type;

						auto it = structs.find(entry_type);

						if (it == structs.end())
							goto skip_struct;

						const auto& entry_struct = it->second;

						if (!ast_entry->name)
						{
							for (auto& entry_struct_entry : entry_struct.entries)
							{
								s.entries.push_back(entry_struct_entry);
							}
						}
						else
						{
							DataField df;
							df.from_ast(*this, *ast_entry);
							s.entries.push_back(df);
						}
					}
					break;

					default:
						throw std::runtime_error("Unions in structs not supported");
				}
			}

			if (!ast_struct->dbe.functions.empty())
				throw std::runtime_error("Functions not supported in structs");

			std::cout << "New struct: " << struct_name << std::endl;
			structs.insert({struct_name, s});
			completed_structs.push_back(struct_name);

			skip_struct:
				;
		}

		for (auto struct_name : completed_structs)
			pending_structs.erase(struct_name);

		completed_structs.clear();
	}

}


}
