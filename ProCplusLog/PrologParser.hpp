#ifndef PROLOG_PARSER_H
#define PROLOG_PARSER_H

#include <istream>
#include <string>

namespace prolog {

	// Prolog parsing library inspirited by SAX
	class prolog_parser {
		std::istream& _input;
		size_t		  _depth = 0;
		size_t		  _line = 1;
		bool		  _forward_skiping = false;

	public:
		// Types of pieces that could be find in prolog
		enum element_type {
			TERM_ARGS_START,	// term**(**arg
			TERM_ARGS_SEP,		// arg1**,**arg2
			TERM_ARGS_END,		// arg**)** or arg **]** aka "end of list"
			RULE_ARGS_START,	// sth **:-**
			RULE_CONJUCTION,	// term1 **,** term2
			TERM,				// **term**
			FACT_END,			//  xxx **.**
			UNKNOWN,
			END_OF_FILE
		};
		// If changed, change Prolog::init() function !!!

		// Constructor
		prolog_parser(std::istream& input_stream);

		prolog_parser(std::istream& input_stream, bool no_skiping);

		// Skips actual element and move to next one
		void skip();

		// Reads actual element and move to next (only make sense in TERM state)
		std::string next();

		// Gets type of actual element
		element_type type() const;

		// Returns actual line number in input stream
		size_t line_number() const;

		// Return next char from input stream without removing
		int peek() const;

		// Reads rest of line aka. [act_position, end_of_line]
		std::string next_line();
	private:
		// Skips whitespace
		void skip_whitespace();

		// Test if char is allowed in term
		bool istermchar(int i) const;
	};

}

#endif // PROLOG_PARSER_H

