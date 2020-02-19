#ifndef COMMON_HPP
#define COMMON_HPP

#include <stdexcept>
#include <string>
#include <variant>

// UOP2 is a hash function which must give a unique value for all operator pairs
// UOP1 is an alias for single character operators
// UOP_ALT is an unused symbol that marks operators with 2 meanings
#define UOP2(a, b) ((unsigned char)((((unsigned char)(a) - 32)) + (((unsigned char)(b)) << 2)))
#define UOP1(a) UOP2(a, '\0')
#define UOP_ALT '\2'

enum OperatorAssoc
{
	ASSOC_NONE,  // For unary and meta-operators (parentheses)
	ASSOC_RIGHT, // x+y+z is (x+y)+z
	ASSOC_LEFT,  // x=y=z is x=(y=z)
};

enum OperatorArgs
{
	OP_UNARY  = 1, // Operator is prefixed to an argument (e.g. -x, !foo)
	OP_BINARY = 2 // Operator sits between 2 arguments (e.g. x-y, foo=bar)
};

enum class Operator : unsigned char
{
	LeftParens       = UOP1('('),
	RightParens      = UOP1(')'),
	BitAnd           = UOP1('&'),
	LogicalAnd       = UOP2('&', '&'),
	BitOr            = UOP1('|'),
	LogicalOr        = UOP2('|', '|'),
	Assign           = UOP1('='),
	Equality         = UOP2('=', '='),
	Inequality       = UOP2('!', '='),
	LessThan         = UOP1('<'),
	LessThanEqual    = UOP2('<', '='),
	GreaterThan      = UOP1('>'),
	GreaterThanEqual = UOP2('>', '='),
	Add              = UOP1('+'),
	Subtract         = UOP1('-'),
	Multiply         = UOP1('*'),
	Divide           = UOP1('/'),
	Modulo           = UOP1('%'),
	BitNot           = UOP1('~'),
	LogicalNot       = UOP1('!'),
	Negate           = UOP2('-', UOP_ALT),
	Increment        = UOP2('+', '+'),
	Decrement        = UOP2('-', '-')
};

struct OperatorInfo
{
	Operator op;         // UOP2 hash
	OperatorArgs args;   // Number of arguments (1 or 2) (see op_args)
	char prec;           // Precedence (0-n)
	OperatorAssoc assoc; // Associativity (see op_assoc)
};

OperatorInfo op_info(Operator op);

using TokenData = std::variant<
	std::string,
	int
>;

struct Token
{
	enum TokenType
	{
		Invalid    = 0,
		Identifier = 1,
		String     = 2,
		Integer    = 4,
		Float      = 8,
		Boolean    = 16,
		Operator   = 32,
		Symbol     = 64,
		NewLine    = 128,
		EndOfFile  = 256,
		Character  = 512
	};

	TokenType type;
	TokenData data;

	int newlines; // Parser uses this to keep track of how many preceeding newline tokens there were

	Token(TokenType type = Invalid, TokenData data = {})
		: type(type)
		, data(data)
		, newlines(0)
	{ }

	explicit operator bool() const
	{
		return this->type != Invalid;
	}

	explicit operator int() const
	{
		return (data.index() == 1) ? std::get<int>(data)
		                           : 0;
	}

	explicit operator std::string() const
	{
		return (data.index() == 0) ? std::get<std::string>(data)
		                           : "";
	}
};

struct Syntax_Error : public std::runtime_error
{
	private:
		int line_;

	public:
		Syntax_Error(const std::string &what_, int line_)
			: runtime_error(what_)
			, line_(line_)
		{ }

		virtual ~Syntax_Error() override;

		int line() const
		{
			return line_;
		}
};

std::string lowercase(const std::string& subject);

#endif // COMMON_HPP
