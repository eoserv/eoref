#ifndef EO_PROTOCOL_HPP
#define EO_PROTOCOL_HPP

#include "ast.hpp"

#include <iosfwd>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

namespace eo_protocol
{

enum Type
{
	TYPE_BYTE,
	TYPE_CHAR,
	TYPE_SHORT,
	TYPE_THREE,
	TYPE_INT,
	TYPE_STRING,
	TYPE_STRING_RAW,
	TYPE_STRUCT
};

struct EO_Protocol;

struct DataField;
struct DataFieldBreak;
struct StructField;
struct UnionBlock;

using DataFieldSize = std::variant<
	std::string,
	int
>;

struct DataField
{
	std::optional<std::string> enum_type;
	std::optional<std::string> struct_type;
	Type type;
	std::optional<std::string> name;
	std::optional<std::string> initializer;
	std::optional<DataFieldSize> size;

	void from_ast(const EO_Protocol& proto, const ::DataField& ast);
	void from_ast(const EO_Protocol& proto, const ::StructField& ast);
};

struct DataFieldBreak
{
};

struct UnionBlock
{

};

using DataBlockEntry = std::variant<
	DataField,
	DataFieldBreak,
	UnionBlock
>;

using DataBlockEntries = std::deque<DataBlockEntry>;

struct Enum
{
	std::optional<std::string> comment;
	std::string base_type;
	std::unordered_map<std::string, int> values;
};

struct Struct
{
	std::optional<std::string> comment;
	DataBlockEntries entries;
};

struct ClientPacket
{
	std::optional<std::string> comment;
	DataBlockEntries entries;
};

struct EO_Protocol
{
	std::unordered_map<std::string, Enum> enums;
	std::unordered_map<std::string, Struct> structs;
	std::unordered_map<std::string, ClientPacket> client_packets;

	void from_ast(const ::ProtocolFile& ast);
};


/*
void write_client_packet_struct(std::ostream& os, const ProtocolFile& ast,
                                const std::string& packet_name,
                                const PacketBlock& packet_block);

void write_client_packet_impl(std::ostream& os, const ProtocolFile& ast,
                              const std::string& packet_name,
                              const PacketBlock& packet_block);

void write_server_packet_struct(std::ostream& os, const ProtocolFile& ast,
                                const std::string& packet_name,
                                const PacketBlock& packet_block);

void write_server_packet_impl(std::ostream& os, const ProtocolFile& ast,
                              const std::string& packet_name,
                              const PacketBlock& packet_block);
*/
}

#endif // EO_PROTOCOL_HPP
