#ifndef EO_PROTOCOL_PARSER_HPP
#define EO_PROTOCOL_PARSER_HPP

#include <functional>
#include <memory>

#include "ast.hpp"
#include "parse.hpp"

class EO_Protocol_Parser
{
	private:
		std::unique_ptr<Parser_Token_Server_Base> tok;
		Token peeked_token;

		static std::size_t hash_str(const std::string& s)
		{
			return std::hash<std::string>()(s);
		}

		std::size_t kw_break = hash_str("break");
		std::size_t kw_enum = hash_str("enum");
		std::size_t kw_struct = hash_str("struct");
		std::size_t kw_union = hash_str("union");
		std::size_t kw_client_packet = hash_str("client_packet");
		std::size_t kw_server_packet = hash_str("server_packet");
		std::size_t kw_fn = hash_str("fn");

		bool GetToken(Token& t, unsigned int allow = 0xFFFFFFFF)
		{
			t = this->tok->GetToken(allow);
			return bool(t);
		}

		bool GetTokenIf(Token& t, std::function<bool(const Token&)> f, unsigned int allow = 0xFFFFFFFF)
		{
			t = this->tok->GetToken(allow);

			if (t && !f(t))
			{
				this->tok->PutBack(t);
				t = peeked_token = {};
			}

			return bool(t);
		}

	public:
		template <class IT> EO_Protocol_Parser(IT begin, IT end)
			: tok(new Parser_Token_Server<IT>(begin, end))
		{ }

		void ParseDataBlockEntries(DataBlockEntries& dbe);

		void ParseDataField(DataField::ptr& data_field);
		void ParseStructField(StructField::ptr& struct_field);

		void ParseEnumEntries(EnumBlock::ptr& enum_block);
		std::string ParseEnumBlock(EnumBlock::ptr& enum_block);

		void ParseStructEntries(StructBlock::ptr& struct_block);
		std::string ParseStructBlock(StructBlock::ptr& struct_block);

		void ParseFunction(PacketFunction::ptr& packet_function);

		void ParseUnionCaseEntries(UnionCase::ptr& union_case);
		std::string ParseUnionCase(UnionCase::ptr& union_case);
		void ParseUnionBlock(UnionBlock::ptr& union_block);

		void ParsePacketEntries(PacketBlock::ptr& packet_block);
		std::string ParsePacketBlock(PacketBlock::ptr& packet_block);

		void ParseProtocolFile(ProtocolFile::ptr& protocol);
};

#endif // EO_PROTOCOL_PARSER_HPP
