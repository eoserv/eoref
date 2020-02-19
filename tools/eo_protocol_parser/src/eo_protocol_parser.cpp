#include "eo_protocol_parser.hpp"

#define PARSER_ERROR(s) throw Parser_Error((s), this->tok->RejectLine())
#define PARSER_ERROR_GOT(s) throw Parser_Error(std::string((s)) + " Got: " + token_got_string((this->tok->RejectToken())), this->tok->RejectLine())

void EO_Protocol_Parser::ParseDataBlockEntries(DataBlockEntries& dbe)
{
	Token t;

	auto is_kw_struct = [this](const Token& t)
		{ return hash_str(std::string(t)) == kw_struct; };

	auto is_kw_union = [this](const Token& t)
		{ return hash_str(std::string(t)) == kw_union; };

	auto is_kw_fn = [this](const Token& t)
		{ return hash_str(std::string(t)) == kw_fn; };

	while (true)
	{
		if (GetTokenIf(t, [](const Token& t) { return std::string(t) == "}"; }, Token::Symbol))
			break;

		if (GetTokenIf(t, is_kw_struct, Token::Identifier))
		{
			StructField::ptr fn;
			ParseStructField(fn);
			dbe.entries.push_back(fn);
		}
		else if (GetTokenIf(t, is_kw_union, Token::Identifier))
		{
			UnionBlock::ptr fn;
			ParseUnionBlock(fn);
			dbe.entries.push_back(fn);
		}
		else if (GetTokenIf(t, is_kw_fn, Token::Identifier))
		{
			PacketFunction::ptr fn;
			ParseFunction(fn);
			dbe.functions.push_back(fn);
		}
		else
		{
			DataField::ptr data_field;
			ParseDataField(data_field);
			dbe.entries.push_back(data_field);
		}
	}
}

void EO_Protocol_Parser::ParseDataField(DataField::ptr& data_field)
{
	Token t;

	data_field = DataField::create();

	if (!this->GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected data field type-name.");

	data_field->type = std::string(t);

	if (data_field->type == "break")
		return;

	if (this->GetTokenIf(t, [](const Token& t) { return std::string(t) == ":"; }, Token::Symbol))
	{
		if (!this->GetToken(t, Token::Identifier))
			PARSER_ERROR_GOT("Expected identifier after ':'.");

		data_field->type_class = std::string(t);
	}

	if (this->GetTokenIf(t, [](const Token& t) { return int(t) == UOP1('('); }, Token::Operator))
	{
		if (!this->GetToken(t, Token::Integer | Token::Identifier))
			PARSER_ERROR_GOT("Expected integer or identifier after '('.");

		if (t.type == Token::Integer)
			data_field->type_static_size = int(t);
		else
			data_field->type_dynamic_size = std::string(t);

		if (!this->GetToken(t, Token::Operator) || int(t) != UOP1(')'))
			PARSER_ERROR_GOT("Expected ')' after type-size-specifier.");
	}

	if (this->GetToken(t, Token::Identifier))
		data_field->name = std::string(t);

	if (GetTokenIf(t, [](const Token& t) { return std::string(t) == "["; }, Token::Symbol))
	{
		if (this->GetToken(t, Token::Integer | Token::Identifier))
		{
			if (t.type == Token::Integer)
				data_field->static_size = int(t);
			else
				data_field->dynamic_size = std::string(t);
		}
		else
		{
			data_field->implicit_size = true;
		}

		if (!this->GetToken(t, Token::Symbol) || std::string(t) != "]")
			PARSER_ERROR_GOT("Expected ']' after size-specifier.");
	}

	if (!data_field->name)
	{
	    if (!GetTokenIf(t, [](const Token& t) { return int(t) == UOP1('='); }, Token::Operator))
			PARSER_ERROR_GOT("Expected '=' after anonymous data field.");

		if (!this->GetToken(t, Token::Integer | Token::Character))
			PARSER_ERROR_GOT("Expected integer or character after '='.");

		if (t.type == Token::Integer)
		{
			data_field->initializer = int(t);
		}
		else
		{
			auto s = std::string(t);

			if (s.empty())
				PARSER_ERROR_GOT("Character constant cannot be empty.");

			if (s.size() > 1)
				PARSER_ERROR_GOT("Character constant must contain exactly one character.");

			data_field->initializer = static_cast<unsigned char>(s[0]);
		}
	}
}

void EO_Protocol_Parser::ParseStructField(StructField::ptr& struct_field)
{
	Token t;

	struct_field = StructField::create();

	if (!this->GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected struct field type-name.");

	struct_field->type = std::string(t);

	if (!this->GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected identifier after struct type-name.");

	struct_field->name = std::string(t);

	if (GetTokenIf(t, [](const Token& t) { return std::string(t) == "["; }, Token::Symbol))
	{
		if (this->GetToken(t, Token::Integer | Token::Identifier))
		{
			if (t.type == Token::Integer)
				struct_field->static_size = int(t);
			else
				struct_field->dynamic_size = std::string(t);
		}
		else
		{
			struct_field->implicit_size = true;
		}

		if (!this->GetToken(t, Token::Symbol) || std::string(t) != "]")
			PARSER_ERROR_GOT("Expected ']' after size-specifier.");
	}
}

void EO_Protocol_Parser::ParseEnumEntries(EnumBlock::ptr& enum_block)
{
	Token t;

	std::string key;
	int value;

	while (this->GetToken(t, Token::Symbol | Token::Integer))
	{
		if (t.type == Token::Symbol)
		{
			if (std::string(t) != "}")
				break;

			return;
		}

		if (t.type != Token::Integer)
			break;

		value = int(t);

		if (!this->GetToken(t, Token::Identifier))
			PARSER_ERROR_GOT("Expected identifier after enum-entry value.");

		key = std::string(t);

		enum_block->entries.insert({key, value});
	}

	PARSER_ERROR_GOT("Expected numeric value for enum-entry or closing brace '}'.");
}

std::string EO_Protocol_Parser::ParseEnumBlock(EnumBlock::ptr& enum_block)
{
	Token t;

	enum_block = EnumBlock::create();

	if (!this->GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected identifier after 'enum'.");

	std::string name = std::string(t);

	while (this->GetToken(t, Token::Symbol))
	{
		if (!enum_block->type_class && std::string(t) == ":")
		{
			if (!this->GetToken(t, Token::Identifier))
				PARSER_ERROR_GOT("Expected type-name after type-classer ':'.");

			enum_block->type_class = std::string(t);
		}
		else if (std::string(t) == "{")
		{
			ParseEnumEntries(enum_block);
			return name;
		}
		else
		{
			break;
		}
	}

	if (enum_block->type_class)
		PARSER_ERROR_GOT("Expected type-classer ':' or opening brace '{' after enum-name.");
	else
		PARSER_ERROR_GOT("Expected opening brace '{' after enum-type-class.");
}

void EO_Protocol_Parser::ParseStructEntries(StructBlock::ptr& struct_block)
{
	ParseDataBlockEntries(struct_block->dbe);
}

std::string EO_Protocol_Parser::ParseStructBlock(StructBlock::ptr& struct_block)
{
	Token t;

	struct_block = StructBlock::create();

	if (!this->GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected identifier after 'struct'.");

	std::string name = std::string(t);

	if (!this->GetToken(t, Token::Symbol) && std::string(t) != "{")
		PARSER_ERROR_GOT("Expected opening brace '{' after struct-name.");

	ParseStructEntries(struct_block);
	return name;
}

void EO_Protocol_Parser::ParseFunction(PacketFunction::ptr& packet_function)
{
	Token t;
	int pre_space = 0;

	auto formatted_append = [&pre_space](const Token& t, std::string& str,
	                                           bool is_signature)
	{
		if (t.type == Token::NewLine)
		{
			if (!is_signature && !str.empty())
			{
				if (str.back() == ' ')
					str.pop_back();

				str += '\n';
				pre_space = 0;
			}
		}
		else if (t.type == Token::Operator)
		{
			auto i = int(t);
			auto info = op_info(static_cast<Operator>(i));

			auto s = op_to_string(i);

			if (info.assoc == ASSOC_NONE)
			{
				str += s;
			}
			else
			{
				// Assume operators will be * or & for signatures
				//   and always arithmetic operators for body.
				if (is_signature)
					str += s + ' ';
				else
					str += ' ' + s + ' ';
			}

			if (is_signature && s == ")")
				++pre_space;
			else
				pre_space = 0;
		}
		else if (t.type == Token::Symbol)
		{
			auto s = std::string(t);
			str += s;
			pre_space = 0;
		}
		else
		{
			auto s = std::string(t);

			if (s.empty())
				s = std::to_string(int(t));

			if (pre_space)
				str += ' ';

			str += s;
			++pre_space;
		}
	};

	packet_function = PacketFunction::create();

	std::string signature;

	while (true)
	{
		if (GetToken(t))
		{
			if (t.type == Token::Symbol && std::string(t) == "{")
				break;

			formatted_append(t, signature, true);
		}
		else
		{
			PARSER_ERROR_GOT("Expected opening brace '{' after fn-signature.");
		}
	}

	if (signature.empty())
		PARSER_ERROR_GOT("Expected fn-signature after 'fn'.");

	if (signature.back() == ' ')
		signature.pop_back(); // trailing space

	int depth = 1;

	std::string body;
	pre_space = 0;

	while (true)
	{
		if (GetToken(t))
		{
			if (t.type == Token::Symbol && std::string(t) == "{")
			{
				++depth;
			}
			else if (t.type == Token::Symbol && std::string(t) == "}")
			{
				--depth;

				if (depth == 0)
					break;
			}
			else
			{
				formatted_append(t, body, false);
			}
		}
		else
		{
			PARSER_ERROR_GOT("Expected closing brace '}' after fn-body.");
		}
	}

	if (body.empty())
		PARSER_ERROR_GOT("Expected fn-body after '{'.");

	if (body.back() == ' ')
		body.pop_back();

	packet_function->signature = std::move(signature);
	packet_function->body = std::move(body);
}


void EO_Protocol_Parser::ParseUnionCaseEntries(UnionCase::ptr& union_case)
{
	ParseDataBlockEntries(union_case->dbe);
}

std::string EO_Protocol_Parser::ParseUnionCase(UnionCase::ptr& union_case)
{
	Token t;

	union_case = UnionCase::create();

	if (!GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected identifier at start of union-case.");

	std::string type = std::string(t);

	if (!GetToken(t, Token::Symbol) || std::string(t) != ":")
		PARSER_ERROR_GOT("Expected ':' after union-case-type.");

	if (!GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected identifier after ':'.");

	union_case->name = std::string(t);

	if (!this->GetToken(t, Token::Symbol) && std::string(t) != "{")
		PARSER_ERROR_GOT("Expected opening brace '{' after union-case.");

	ParseUnionCaseEntries(union_case);
	return type;
}

void EO_Protocol_Parser::ParseUnionBlock(UnionBlock::ptr& union_block)
{
	Token t;

	union_block = UnionBlock::create();

	if (!this->GetToken(t, Token::Operator) && int(t) != UOP1('('))
		PARSER_ERROR_GOT("Expected opening parenthesis '(' after 'union'.");

	if (!this->GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected identifier after opening parenthesis '('.");

	union_block->switch_field = std::string(t);

	if (!this->GetToken(t, Token::Operator) && int(t) != UOP1(')'))
		PARSER_ERROR_GOT("Expected closing parenthesis ')' after union-switch-field.");

	if (!this->GetToken(t, Token::Symbol) && std::string(t) != "{")
		PARSER_ERROR_GOT("Expected opening brace '{' after union-specifier.");

	while (true)
	{
		if (GetTokenIf(t, [](const Token& t) { return std::string(t) == "}"; }, Token::Symbol))
			break;

		UnionCase::ptr union_case;
		std::string type = ParseUnionCase(union_case);
		union_block->cases.insert({type, union_case});
	}
}

void EO_Protocol_Parser::ParsePacketEntries(PacketBlock::ptr& packet_block)
{
	ParseDataBlockEntries(packet_block->dbe);
}

std::string EO_Protocol_Parser::ParsePacketBlock(PacketBlock::ptr& packet_block)
{
	Token t;

	std::string name;
	packet_block = PacketBlock::create();

	if (this->GetToken(t, Token::Identifier))
		name = std::string(t);

	if (!this->GetToken(t, Token::Operator) && int(t) != UOP1('('))
		PARSER_ERROR_GOT("Expected opening parenthesis '(' after packet-keyword or packet-name.");

	if (!this->GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected identifier after opening parenthesis '('.");

	packet_block->family = std::string(t);

	if (!this->GetToken(t, Token::Symbol) && std::string(t) != ",")
		PARSER_ERROR_GOT("Expected comma ',' after packet-family.");

	if (!this->GetToken(t, Token::Identifier))
		PARSER_ERROR_GOT("Expected identifier after comma ','.");

	packet_block->action = std::string(t);

	if (!this->GetToken(t, Token::Operator) && int(t) != UOP1(')'))
		PARSER_ERROR_GOT("Expected closing parenthesis ')' after packet-action.");

	if (!this->GetToken(t, Token::Symbol) && std::string(t) != "{")
		PARSER_ERROR_GOT("Expected opening brace '{' after packet-type-specifier.");

	if (name.empty())
		name = packet_block->family + "_" + packet_block->action;

	ParsePacketEntries(packet_block);
	return name;
}

void EO_Protocol_Parser::ParseProtocolFile(ProtocolFile::ptr& protocol)
{
	Token t;

	std::optional<std::string> string_comment;

	protocol = ProtocolFile::create();

	while (this->GetToken(t, Token::String | Token::Identifier | Token::EndOfFile))
	{
		switch (t.type)
		{
			case Token::String:
				string_comment = std::string(t);
				break;

			case Token::Identifier:
			{
				auto h = hash_str(std::string(t));

				if (h == kw_enum)
				{
					EnumBlock::ptr enum_block;
					auto name = ParseEnumBlock(enum_block);
					enum_block->comment = string_comment;
					string_comment = std::nullopt;
					protocol->enums.insert({name, enum_block});
				}
				else if (h == kw_struct)
				{
					StructBlock::ptr struct_block;
					auto name = ParseStructBlock(struct_block);
					struct_block->comment = string_comment;
					string_comment = std::nullopt;
					protocol->structs.insert({name, struct_block});
				}
				else if (h == kw_client_packet)
				{
					PacketBlock::ptr packet_block;
					auto name = ParsePacketBlock(packet_block);
					packet_block->comment = string_comment;
					string_comment = std::nullopt;
					protocol->client_packets.insert({name, packet_block});
				}
				else if (h == kw_server_packet)
				{
					PacketBlock::ptr packet_block;
					auto name = ParsePacketBlock(packet_block);
					packet_block->comment = string_comment;
					string_comment = std::nullopt;
					protocol->server_packets.insert({name, packet_block});
				}
				else
				{
					goto fail;
				}
			}
			break;

			case Token::EndOfFile:
				return;
		}
	}

fail:
	PARSER_ERROR_GOT("Expected string-comment or block-identifier (enum/struct/client_packet/server_packet) in global scope.");
}
