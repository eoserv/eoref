#ifndef PARSE_HPP
#define PARSE_HPP

#include "common.hpp"

#include <stack>

class Parser_Token_Server_Base
{
	private:
		std::stack<Token> token_buffer;
		int line;

		Token reject_token;
		int reject_line;

	protected:
		virtual Token xGetToken() = 0;

	public:
		Parser_Token_Server_Base();

		Token GetToken(unsigned int allow = 0xFFFFFFFF);
		void PutBack(Token);

		Token RejectToken() const { return this->reject_token; }
		int RejectLine() const { return std::max(this->line, this->reject_line); }

		int Line() const { return this->line; }

		virtual ~Parser_Token_Server_Base();
};

template <class IT> class Parser_Token_Server : public Parser_Token_Server_Base
{
	public:
		typedef IT iterator_type;

	private:
		IT it;
		IT end;

	protected:
		Token xGetToken()
		{
			if (it == end)
				return Token();

			return *(it++);
		}

	public:
		Parser_Token_Server(IT begin, IT end)
			: it(begin)
			, end(end)
		{ }
};

std::string op_to_string(unsigned char h);
std::string cut_string(std::string s);
std::string token_got_string(Token t);

struct Parser_Error : public Syntax_Error
{
	public:
		Parser_Error(const std::string &what_, int line_)
			: Syntax_Error(what_, line_)
		{ }

		virtual ~Parser_Error() override;
};

#endif // PARSE_HPP
