#include "parse.hpp"

Parser_Token_Server_Base::Parser_Token_Server_Base()
	: line(1)
	, reject_line(1)
{ }

Token Parser_Token_Server_Base::GetToken(unsigned int allow)
{
	Token t;
	int newlines = 0;

	while (true)
	{
		if (!this->token_buffer.empty())
		{
			t = this->token_buffer.top();
			this->token_buffer.pop();
		}
		else
		{
			t = this->xGetToken();
		}

		if (t.type == Token::NewLine)
		{
			++this->line;

			if ((Token::NewLine & allow) == Token::NewLine)
				break;

			++newlines;
		}
		else
		{
			break;
		}
	}

	t.newlines = newlines;

	if ((t.type & allow) != t.type)
	{
		this->PutBack(t);
		return Token();
	}

	return t;
}

void Parser_Token_Server_Base::PutBack(Token t)
{
	this->reject_token = t;
	this->reject_line = this->line;
	this->line -= t.newlines;

	if (t.type == Token::NewLine)
		--this->line;

	while (t.newlines-- > 0)
		this->token_buffer.push(Token(Token::NewLine));

	this->token_buffer.push(t);
}

Parser_Token_Server_Base::~Parser_Token_Server_Base()
{ }

std::string op_to_string(unsigned char h)
{
	switch (h)
	{
		case UOP1('('):          return "(";
		case UOP1(')'):          return ")";
		case UOP1('&'):          return "&";
		case UOP2('&', '&'):     return "&&";
		case UOP1('|'):          return "|";
		case UOP2('|', '|'):     return "||";
		case UOP1('='):          return "=";
		case UOP2('=', '='):     return "==";
		case UOP2('!', '='):     return "!=";
		case UOP1('<'):          return "<";
		case UOP2('<', '='):     return "<=";
		case UOP1('>'):          return ">";
		case UOP2('>', '='):     return ">=";
		case UOP1('+'):          return "+";
		case UOP1('-'):          return "-";
		case UOP1('*'):          return "*";
		case UOP1('/'):          return "/";
		case UOP1('%'):          return "%";
		case UOP1('~'):          return "~";
		case UOP1('!'):          return "!";
		case UOP2('-', UOP_ALT): return "-";
		case UOP2('+', '+'):     return "++";
		case UOP2('-', '-'):     return "--";
		default:                 return "(unknown)";
	}
}

std::string cut_string(std::string s)
{
	if (s.length() > 16)
		return s.substr(0, 13) + "...";

	return s;
}

std::string token_got_string(Token t)
{
	switch (t.type)
	{
		case Token::Invalid:    return "invalid token";
		case Token::Identifier: return "identifier '" + std::string(t) + "'";
		case Token::String:     return "string '" + cut_string(std::string(t)) + "'";
		case Token::Integer:    return "integer '" + std::to_string(int(t)) + "'";
		case Token::Float:      return "float '" + std::string(t) + "'";
		case Token::Boolean:    return "boolean '" + std::string(int(t) ? "true" : "false") + "'";
		case Token::Operator:   return "operator '" + op_to_string(int(t)) + "'";
		case Token::Symbol:     return "symbol '" + std::string(t) + "'";
		case Token::NewLine:    return "new-line";
		case Token::EndOfFile:  return "end of file";
		case Token::Character:  return "character '" + cut_string(std::string(t)) + "'";
	}

	return "unknown token";
}

Parser_Error::~Parser_Error()
{ }
