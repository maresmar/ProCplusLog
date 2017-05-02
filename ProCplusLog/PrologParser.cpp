#include "PrologParser.hpp"
#include <assert.h>
#include <sstream>

using namespace prolog;

// class prolog_parser definition

// Constructor
prolog_parser::prolog_parser(std::istream& input_stream) : _input(input_stream) {
	skip_whitespace();
};

prolog_parser::prolog_parser(std::istream& input_stream, bool forward_skiping) : _input(input_stream), _forward_skiping(forward_skiping) {
	if(!forward_skiping)
		skip_whitespace();
};

// Skips actual element and move to next one
void prolog_parser::skip() {
	if (_forward_skiping)
		skip_whitespace();
	switch (type())
	{
	case TERM_ARGS_START:
		_input.ignore();
		_depth++;
		break;
	case TERM_ARGS_END:
		if(_input.peek()!=']')
			_depth--;
		_input.ignore();
		break;
	case RULE_ARGS_START:
		assert(_depth == 0);
		_input.ignore(2);
		break;
	case RULE_CONJUCTION:
	case TERM_ARGS_SEP:
	case FACT_END:
		_input.ignore();
		break;
	case TERM:
		if (_input.peek() == '[')
			_input.ignore();
		else
			while (istermchar(_input.peek())) {
				_input.ignore();
			}
		break;
	default:
		assert(false);
		exit(10);
	}
	if(!_forward_skiping)
		skip_whitespace();
}

// Reads actual element and move to next (only make sense in TERM state)
std::string prolog_parser::next() {
	assert(type() == element_type::TERM);
	if (_forward_skiping)
		skip_whitespace();
	std::stringstream ss;
	while (istermchar(_input.peek())) {
		ss << (char) _input.get();
	}
	if (!_forward_skiping)
		skip_whitespace();
	return ss.str();
}

// Gets type of actual element
prolog_parser::element_type prolog_parser::type() const {
	if (_forward_skiping)
		const_cast<prolog_parser*>(this)->skip_whitespace();
	int ch = _input.peek();
	if (ch == EOF)
		return element_type::END_OF_FILE;
	if (istermchar(ch)) { // note '[' is considered as (special) term
		return element_type::TERM;
	}
	else {
		switch (ch)
		{
		case '.':
			return element_type::FACT_END;
		case '(':
			return element_type::TERM_ARGS_START;
		case ')':
		case ']':
			return element_type::TERM_ARGS_END;
		case '|':
		case ',':
			if (_depth == 0)
				return element_type::RULE_CONJUCTION;
			else
				return element_type::TERM_ARGS_SEP;
		case ':': {
			_input.ignore();
			int next_char = _input.peek();
			_input.putback(ch);
			if (next_char == '-')
				return element_type::RULE_ARGS_START;
			else // or next_char is EOF
				return element_type::UNKNOWN;
		}
		default:
			return element_type::UNKNOWN;
		}
	}
}

// Returns actual line number in input stream
size_t prolog_parser::line_number() const {
	return _line;
}

// Skips whitespace
void prolog_parser::skip_whitespace() {
	while (isspace(_input.peek()) || _input.peek() == '%') {
		if (_input.peek() == '%') { 
			int ch;
			while ((ch = _input.get()) != EOF) {
				if (iscntrl(ch))
					break;
			}
			++_line;
		} else if (iscntrl(_input.get()))
			++_line;
	}
}

// Test if char is allowed in term
bool prolog_parser::istermchar(int ch) const {
	if (isalnum(ch))
		return true;
	else 
		switch (ch)
		{
		case '_':
		case '[':
		//note ']' is considered as TERM_ARGS_END
		case '!':
			return true;
		default:
			return false;
		}
}

int prolog_parser::peek() const {
	return _input.peek();
}

// Reads rest of line aka. [act_position, end_of_line]
std::string prolog_parser::next_line() {
	std::string out;
	std::getline(_input, out);
	return std::move(out);
}