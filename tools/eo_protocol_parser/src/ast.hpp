#ifndef AST_HPP
#define AST_HPP

#include <deque>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>

template <class T>
struct ASTNode
{
	using ptr = std::shared_ptr<T>;

	template <class... Args>
	static ptr create(Args&&... args)
	{ return std::make_shared<T>(std::forward(args)...); }
};

struct PacketFunction;
struct DataBlockEntries;
struct DataField;
struct StructField;
struct UnionCase;
struct UnionBlock;
struct EnumBlock;
struct StructBlock;
struct PacketBlock;
struct ProtocolFile;

using PacketBlockEntry = std::variant<
	std::shared_ptr<DataField>,
	std::shared_ptr<StructField>,
	std::shared_ptr<UnionBlock>
>;

struct PacketFunction : ASTNode<PacketFunction>
{
	std::string signature;
	std::string body;
};

struct DataBlockEntries
{
	std::deque<PacketBlockEntry> entries;
	std::deque<PacketFunction::ptr> functions;
};

struct DataField : ASTNode<DataField>
{
	std::string type;
	std::optional<std::string> type_class;
	std::optional<int> type_static_size;
	std::optional<std::string> type_dynamic_size;
	std::optional<std::string> name;
	std::optional<int> initializer;
	std::optional<int> static_size;
	std::optional<std::string> dynamic_size;
	bool implicit_size = false;
};

struct StructField : ASTNode<StructField>
{
	std::string type;
	std::optional<std::string> name;
	std::optional<int> static_size;
	std::optional<std::string> dynamic_size;
	bool implicit_size = false;
};

struct UnionCase : ASTNode<UnionCase>
{
	std::string name;
	DataBlockEntries dbe;
};

struct UnionBlock : ASTNode<UnionBlock>
{
	std::string switch_field;
	std::unordered_map<std::string, UnionCase::ptr> cases;
};

struct EnumBlock : ASTNode<EnumBlock>
{
	std::optional<std::string> comment;
	std::optional<std::string> type_class;
	std::unordered_map<std::string, int> entries;
};

struct StructBlock : ASTNode<StructBlock>
{
	std::optional<std::string> comment;
	DataBlockEntries dbe;
};

struct PacketBlock : ASTNode<PacketBlock>
{
	std::optional<std::string> comment;
	std::string family;
	std::string action;
	DataBlockEntries dbe;
};

struct ProtocolFile : ASTNode<ProtocolFile>
{
	std::unordered_map<std::string, EnumBlock::ptr> enums;
	std::unordered_map<std::string, StructBlock::ptr> structs;

	std::unordered_map<std::string, PacketBlock::ptr> client_packets;
	std::unordered_map<std::string, PacketBlock::ptr> server_packets;
};

#endif // AST_HPP
