#include "PrologParser.hpp"
#include "ProCplusLog.hpp"

#include <iostream>
#include <fstream>
#include <map>

// Prolog data types
using functor_type = std::string;
using database = prolog::database<functor_type>;
using term = prolog::term<functor_type>;
using composed_term = prolog::composed_term<functor_type>;
using variable_term = prolog::variable_term<functor_type>;
using rule = prolog::rule<functor_type>;
using solver = prolog::solver<functor_type>;

// Parsing variables
using el_t = prolog::prolog_parser::element_type;
using variable_map = std::map<std::string, variable_term>;
enum parsing_state_t { RULE_HEAD };
std::map<el_t, std::string> _element_names;
bool _first_time = true;
bool _debug = false;

// Forward declaration
term read_term(prolog::prolog_parser& parser, variable_map& map);

// Tests equality and if differs prints error and exit
void expected(el_t expectation, el_t found) {
	if (expectation != found) {
		std::cerr << "excepted " << _element_names[expectation];
		std::cerr << ", but " << _element_names[found] << " found" << std::endl;
		exit(2);
	}
}

// Tests equality and if differs prints error and exit
void expected(el_t expectation, prolog::prolog_parser& parser) {
	const el_t found = parser.type();
	if (expectation != found) {
		std::cerr << "[line " << parser.line_number() << " ending] " << parser.next_line() << std::endl;
		expected(expectation, found);
	}
}

// Loads some default data into memory
void init() {
	_element_names[el_t::TERM_ARGS_START] = "start of term args (eg. 'xxx(xxx')";
	_element_names[el_t::TERM_ARGS_SEP] = "args separator (eg. 'xxx,xxx')";
	_element_names[el_t::TERM_ARGS_END] = "ending of term args (eg. 'xxx)')";
	_element_names[el_t::RULE_ARGS_START] = "start of rules body (eg. 'xxx :- xxx')";
	_element_names[el_t::RULE_CONJUCTION] = "rule args conjunction (eg. 'xxx,xxx')";
	_element_names[el_t::TERM] = "term (eg. 'xxx', 'Xxx', '_', '[' or '!')";
	_element_names[el_t::FACT_END] = "ending of facts (eg. 'xxx.')";
	_element_names[el_t::UNKNOWN] = "unknown piece (eg. unsupported characters)";
	_element_names[el_t::END_OF_FILE] = "end of file";
}

// Reads one variable from parser (variable with same name must be copy of same object, because of unification)
variable_term read_variable_term(prolog::prolog_parser& parser, variable_map& map) {
	expected(el_t::TERM, parser);
	std::string name = parser.next();

	if(name != "_") { // '_' is anonymous variable	
		auto found = map.find(name);
		if (found != map.end())
			return found->second.shallow_copy();
		else {
			variable_term var(name);
			map.emplace(name, var.shallow_copy());
			return std::move(var);
		}
	}
	else
		return variable_term(name);
}

// Reads list to the end - parser could be in '[' or it could be recursion ', ...'.
composed_term read_list(prolog::prolog_parser& parser, variable_map& map) {
	if (parser.peek() == ',') {
		parser.skip();
	}
	else {
		expected(el_t::TERM, parser);
		parser.skip(); // skips functor '['
	}
	composed_term term("[");
	if (parser.type() != el_t::TERM_ARGS_END) { // list is not empty eg. []
		term.add_arg(read_term(parser, map));
		if (parser.type() != el_t::TERM_ARGS_END) {  // list has more than one element eg. [xxx, ....] or [ xxx | yyy ]
			if (parser.peek() == '|') { // eg [ xxx | yyy ]
				parser.skip();
				term.add_arg(read_term(parser, map));
				expected(el_t::TERM_ARGS_END, parser);
				parser.skip();
			}
			else { // list is something like [ arg1, ... ]
				expected(el_t::TERM_ARGS_SEP, parser);
				term.add_arg(read_list(parser, map));
			}
		}
		else { // list is [ xxx ]
			expected(el_t::TERM_ARGS_END, parser);
			parser.skip();
			// so add [] to tail
			term.add_arg(composed_term("["));
		}
	} else { // list is []
		expected(el_t::TERM_ARGS_END, parser);
		parser.skip();
	}
	return std::move(term);
}

composed_term read_componed_term(prolog::prolog_parser& parser, variable_map& map) {
	expected(el_t::TERM, parser);
	composed_term term(parser.next());
	if (parser.type() == el_t::TERM_ARGS_START) { // eg. functor(args.... if not term is 
		parser.skip();
		while (parser.type() != el_t::TERM_ARGS_END) {
			expected(el_t::TERM, parser);
			term.add_arg(read_term(parser, map));
			if (parser.type() == el_t::TERM_ARGS_SEP)
				parser.skip();
			else
				assert(parser.type() == el_t::TERM_ARGS_END);
		}
		expected(el_t::TERM_ARGS_END, parser);
		parser.skip();
	}
	return std::move(term);
}

term read_term(prolog::prolog_parser& parser, variable_map& map) {
	expected(el_t::TERM, parser);
	char ch = parser.peek();
	if (isupper(ch) || ch == '_') // Variable
		return read_variable_term(parser, map);
	else if (ch == '[') // List
		return read_list(parser, map); 
	else // Composed term
		return read_componed_term(parser, map);
}

void read_rule_body(prolog::prolog_parser& parser, rule& rule, variable_map& map) {
	// Read each arg of body
	while (parser.type() != el_t::FACT_END) {
		expected(el_t::TERM, parser);
		rule.add_term(read_componed_term(parser, map));
		if (parser.type() == el_t::RULE_CONJUCTION) {
			parser.skip();
		}
	}
}

// Loads prolog database file into memory
database load_db(std::ifstream& ifs) {
	database db;
	prolog::prolog_parser parser(ifs);

	// prolog::rule<> act_rule;
	std::unique_ptr<rule> rule_ptr;

	// Read rules
	while (parser.type() != el_t::END_OF_FILE) {
		variable_map map;
		// Read head of rule
		expected(el_t::TERM, parser);
		rule_ptr = std::make_unique<rule>(read_componed_term(parser, map));
			
		// Read body of rule
		if (parser.type() != el_t::FACT_END) { // With body
			expected(el_t::RULE_ARGS_START, parser);
			parser.skip();
			read_rule_body(parser, *rule_ptr, map);
		}
		expected(el_t::FACT_END, parser);
		parser.skip();
		db.add_rule(std::move(rule_ptr));
	}
	return std::move(db);
}

// Interactively finds solution 
void find_soulition(solver& sol) {
	while (true)
	{
		if (sol.next()) { // If there is solution
			if (sol.solution().cbegin() == sol.solution().cend()) {
				std::cout << "true" << std::endl;
			}
			else {
				std::cout << sol << std::endl;
			}
			// Interaction with the user
			std::cout << "(ENTER) - next solution \t (stop+ENTER) - stop finding solution" << std::endl;
			std::string line;
			std::getline(std::cin, line);
			if (line == "stop")
				break;
		}
		else { // If there isn't solution
			std::cout << "false" << std::endl;
			break;
		}
	}
}

// Program's entry point
int main(int argc, char *argv[])
{
	// Args check
	if (argc != 2) {
		std::cerr << "usage: ";
		std::cerr << argv[0];
		std::cerr << " DB_FILE" << std::endl;
		return 1;
	}

	std::cout << "  ProCplusLog interpreter  " << std::endl;
	std::cout << "===========================" << std::endl;

	// Opens file
	std::ifstream db_file(argv[1], std::ifstream::in);
	if (!db_file.is_open()) {
		std::cerr << "Unable to open file ";
		std::cerr << argv[1] << std::endl;
		return 1;
	}

	// Loads error messages
	init();

	// Parse input db file
	database db = load_db(db_file);

	db_file.close();

	// Starts input/output console
	solver sol(db);
	while(true)  {
		if (_first_time) {
			std::cout << "write terms separated by comma and ended by dot" << std::endl;
			std::cout << "(list.) - list database \t (debug.) - debug on/off \t (halt.) - quit" << std::endl;
			_first_time = false;
		}
		// Reads terms from user
		std::cout << "?-" << std::flush;
		prolog::prolog_parser parser(std::cin, true);
		rule goal(composed_term("goal"));
		read_rule_body(parser, goal, variable_map());
		expected(el_t::FACT_END, parser);
		parser.skip();
		// Skip rest of the line...
		std::string line;
		std::getline(std::cin, line);
		if (goal.begin() == goal.end())
			continue;

		// Proceed input
		if (prolog::unify(*goal.begin(), composed_term("list"), prolog::result_bindings<>())) { // Print db
			std::cout << db << std::endl;
		}
		else if (prolog::unify(*goal.begin(), composed_term("halt"), prolog::result_bindings<>())) { // Exit app
			return 0;
		}
		else if (prolog::unify(*goal.begin(), composed_term("debug"), prolog::result_bindings<>())) { // Toggle debug mode
			if (_debug)
				_debug = false;
			else
				_debug = true;
			sol.set_debug(_debug);
		} else {
			// Try to find solution
			sol.set_goal(std::move(goal));
			find_soulition(sol);
		}
	}

	return 0;
}