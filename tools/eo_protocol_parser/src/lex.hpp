#ifndef LEX_HPP
#define LEX_HPP

#include "common.hpp"

#include <functional>
#include <iosfwd>
#include <queue>

class Lexer
{
	private:
		int row;
		int col;
		std::basic_istream<char>& is;
		std::queue<Token> token_buffer;

		bool PeekChar(char& c);
		bool GetChar(char& c);
		bool GetCharIf(char& c, std::function<bool(char)> f);

		Token ReadNumber();
		Token ReadString();
		Token ReadCharacter();
		Token ReadIdentifier();
		Token ReadSymbol();
		Token ReadOperator();

	public:
		Lexer(std::basic_istream<char>& is);

		Token ReadToken();

		template <class IT> IT Lex(IT it)
		{
			Token t;

			for (t = ReadToken(); t && t.type != Token::EndOfFile; t = ReadToken())
				*it++ = t;

			*it++ = t;

			return it;
		}
};

struct Lexer_Error : public Syntax_Error
{
	private:
		int col_;

	public:
		Lexer_Error(const std::string &what_, int line_, int col_)
			: Syntax_Error(what_, line_)
			, col_(col_)
		{ }

		virtual ~Lexer_Error() override;

		int col() const
		{
			return col_;
		}
};

#endif // LEX_HPP
