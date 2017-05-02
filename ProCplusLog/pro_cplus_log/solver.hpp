// Finds solution for rule in database

#ifndef PRO_CPLUS_LOG_SOLVER_H
#define PRO_CPLUS_LOG_SOLVER_H

// Contains rules management

#include "base.hpp"
#include "rule.hpp"
#include "tools.hpp"
#include<assert.h>
#include<string>
#include<memory>
#include<iostream>

namespace prolog {

	// Prolog call solver
	template<typename T = default_value_type> class solver {
		using db_const_iterator = typename database<T>::const_iterator;
		using rule_const_iterator = typename rule<T>::const_iterator;
		using new_old_var_map = typename term<T>::old_new_varibles_map;

		enum solving_status_t { UNSOLVED, SOLVING_FORWARD, SOLVING_ROLLBACK, SOLVED, NO_SOLUTION };

		// One level on call stack - similar to local variables in recursive function
		class call_stack_entry {
			const call_stack_entry* _parent;	// call_stack_entry that has this term inside its args
			const rule<T>*			_goal;		// Used only in root element
		public:
			const rule_const_iterator				arg_it;			// iterator on actual inspected rule element (to arg)
			const result_bindings<T>				orig_bindings;  // binding valid to actual inspected arg
			const std::shared_ptr<new_old_var_map>	orig_var_changelog_ptr; // map of changes variables when copping (eg X -> X')
			db_const_iterator						found_rule_it;	// iterator to actual inspected rule
			db_const_iterator						found_rule_it_cend;	// last rule with same signature (could be also earlier rule because of Prolog cut aka "!").

			// Create new standard call_stack_etry for going to next arg (from child)
			call_stack_entry(rule_const_iterator&& arg_it, db_const_iterator&& rule_it, db_const_iterator&& rule_it_cend, result_bindings<T>&& bindings, const call_stack_entry& child) :
				arg_it(std::move(arg_it)), found_rule_it(std::move(rule_it)), found_rule_it_cend(rule_it_cend),
				_parent(child.parent_of_parent()), _goal(child.parent()._goal), orig_bindings(std::move(bindings)), 
				orig_var_changelog_ptr(child.parent().orig_var_changelog_ptr) {}

			// Create new standard call_stack_etry for going to next arg (after cut)
			call_stack_entry(rule_const_iterator&& arg_it, db_const_iterator&& rule_it, db_const_iterator&& rule_it_cend, const call_stack_entry& neiberhood) :
				arg_it(std::move(arg_it)), found_rule_it(std::move(rule_it)), found_rule_it_cend(rule_it_cend),
				_parent(neiberhood._parent), _goal(neiberhood._goal), orig_bindings(neiberhood.orig_bindings) ,
				orig_var_changelog_ptr(neiberhood.orig_var_changelog_ptr) {}

			// Create new standard call_stack_etry for going to step in
			call_stack_entry(rule_const_iterator&& arg_it, db_const_iterator&& rule_it, db_const_iterator&& rule_it_cend, result_bindings<T>&& bindings, std::shared_ptr<new_old_var_map>&& var_changel_ptr, const call_stack_entry* parent) :
				arg_it(std::move(arg_it)), found_rule_it(std::move(rule_it)), found_rule_it_cend(rule_it_cend),
				_parent(parent), orig_bindings(std::move(bindings)), orig_var_changelog_ptr(std::move(var_changel_ptr)) {
			}

			// Special constructor (for root)
			call_stack_entry(db_const_iterator&& rule_it, db_const_iterator&& rule_it_cend, rule<T>& goal) :
				arg_it(goal.cbegin()), found_rule_it(std::move(rule_it)), found_rule_it_cend(std::move(rule_it_cend)),
				_parent(NULL), orig_bindings(), orig_var_changelog_ptr(std::make_shared<new_old_var_map>()) {
				_goal = &goal;
			}

			inline bool is_root() const {
				return _parent == NULL;
			}

			// Last argument in actual rule
			inline rule_const_iterator arg_it_cend() const {
				if (is_root()) {
					return _goal->cend();
				}
				else {
					return parent().found_rule_it->cend();
				}
			}

			// Parent call_stack_entry that has this term inside its args
			inline call_stack_entry& parent() const {
				assert(!is_root());
				return *const_cast<call_stack_entry*>(this->_parent);
			}

		private:
			inline call_stack_entry* parent_of_parent() const {
				assert(!is_root());
				return const_cast<call_stack_entry*>(this->_parent->_parent);
			}
		};

		const database<T>& _db;
		rule<T> _goal;

		std::stack<call_stack_entry> _call_stack;
		solving_status_t _solving_status = UNSOLVED;
		result_bindings<T> _results;
		bool _debug = false;


	public:
		// Create new solver with db and goal to resolving
		solver(const database<T>& db, const rule<T>& goal) : _db(db), _goal(goal) {}
		// Create new solver with db and goal to resolving
		solver(const database<T>& db, rule<T>&& goal) : _db(db), _goal(std::move(goal)) {}
		// Create new solver with db and default goal (you should set_goal later)
		solver(const database<T>& db) : _db(db), _goal(composed_term<T>("")) {}

		// Sets goal for resolving
		void set_goal(const rule<T>& r) {
			_solving_status = UNSOLVED;
			_goal = rule<T>(r);
			_call_stack = std::stack<call_stack_entry>();
		}

		// Sets goal for resolving
		void set_goal(rule<T>&& r) {
			_solving_status = UNSOLVED;
			_goal = std::move(r);
			_call_stack = std::stack<call_stack_entry>();
		}

		// Prints debug information on std::cout during solving
		void set_debug(bool debug) {
			_debug = debug;
		}

		// Finds next solution of term with respect to db
		bool next() {
			assert(_solving_status == UNSOLVED || _solving_status == SOLVED);

			// Is it first `next()` calling with this goal?
			if (_solving_status == UNSOLVED) {
				_call_stack.emplace(_db.lower_bound(*_goal.begin()), _db.upper_bound(*_goal.begin()), _goal);
				_solving_status = SOLVING_FORWARD;
			}
			else {
				assert(_solving_status = SOLVED);
				_solving_status = SOLVING_ROLLBACK;
			}
			// Finding of next result
			while (!_call_stack.empty()) {
				switch (_solving_status)
				{
				case SOLVING_FORWARD: {
					call_stack_entry& entry = _call_stack.top();
					new_old_var_map& orig_var_changelog = *entry.orig_var_changelog_ptr;
					const result_bindings<T>& orig_bindings = entry.orig_bindings;
					// Is actual rule finished? (is all args done?)
					if (entry.arg_it != entry.arg_it_cend()) {
						// There are still some args to proceed
						if (entry.arg_it->functor() == "!") {
							// I have to do prolog cut
							entry.parent().found_rule_it_cend = entry.parent().found_rule_it;
							// And let's move to another argument
							rule_const_iterator _arg_it = entry.arg_it;
							++_arg_it;
							// Finds rules in database for next arg
							db_const_iterator db_lower_it, db_upper_it;
							if (_arg_it != entry.arg_it_cend()) {
								db_lower_it = _db.lower_bound(*_arg_it);
								db_upper_it = _db.upper_bound(*_arg_it);
							}
							_call_stack.emplace(std::move(_arg_it), std::move(db_lower_it), std::move(db_upper_it), entry);
							if (_debug) {
								std::cout << "cut proceed (aka !)" << std::endl;
							}
							continue;
						} 
						// Let's find some unifiable rule for actual argument
						// Do a deep copy of term with respect to old changes in actual rule
						// (I need to change it later and because of recursive unification)
						composed_term<T> orig_arg = entry.arg_it->deep_copy(orig_var_changelog);
						// Updates term (puts value instated of known variables)
						orig_bindings.bind_results(orig_arg);

						// Checks rules in database if could be unified to act_arg
						for (;entry.found_rule_it != entry.found_rule_it_cend;
							++entry.found_rule_it) {
							// Prepare head of rule for unification
							std::shared_ptr<new_old_var_map> new_var_changelog_ptr = std::make_shared<new_old_var_map>();
							composed_term<T> new_head = entry.found_rule_it->head().deep_copy(*new_var_changelog_ptr);
							// Try unify
							result_bindings<T> new_bindings;
							if (_debug) {
								print_spaces();
								std::cout << "try unify " << orig_arg << " with " << new_head;
							}
							if (unify(orig_arg, new_head, new_bindings)) {
								// I will test if rule is fact or not later (using semantics of processing post last argument)
								// Finds rules in database for next arg
								db_const_iterator db_lower_it, db_upper_it;
								if(entry.found_rule_it->cbegin() != entry.found_rule_it->cend()) {
									db_lower_it = _db.lower_bound(*entry.found_rule_it->cbegin());
									db_upper_it = _db.upper_bound(*entry.found_rule_it->cbegin());
								}
								_call_stack.emplace(entry.found_rule_it->cbegin(), std::move(db_lower_it), std::move(db_upper_it), std::move(new_bindings), std::move(new_var_changelog_ptr), &entry); // Do recursively same thing
								if(_debug) {
									std::cout << " [OK]" << std::endl;
								}
								break;
							}
							else if (_debug) {
								std::cout << " [FAILED]" << std::endl;
							}
						} 
						// If any unifiable rule found
						if (entry.found_rule_it == entry.found_rule_it_cend) {
							_solving_status = SOLVING_ROLLBACK; // then rollback and find another rule in parent
						}
						break;
					}
					else {
						// There are no more args to proceed in this rule
						// Merge results_bindings together (from child to parent)
						result_bindings<T> new_bindings = entry.parent().orig_bindings;
						// I have to make backup of bindings (in case that unification fail later and I must do rollback)
						new_bindings.merge(entry.orig_bindings, *entry.parent().orig_var_changelog_ptr);
						// Let's proceed with next arg in parent			
						if (_debug) {
							composed_term<T> term = entry.parent().arg_it->deep_copy(*entry.parent().orig_var_changelog_ptr);
							new_bindings.bind_results(term);
							print_spaces();
							std::cout << "merged to " << term << std::endl;
						}
						rule_const_iterator parent_arg_it = entry.parent().arg_it;
						++parent_arg_it;
						// Test if I finished whole unification
						if (entry.parent().is_root() && parent_arg_it == entry.parent().arg_it_cend()) {
							new_bindings.bind_inside();
							_solving_status = SOLVED;
							_results = std::move(new_bindings);
							return true;
						}
					
						// Finds rules in database for next arg
						db_const_iterator db_lower_it, db_upper_it;
						if (parent_arg_it != entry.parent().arg_it_cend()) {
							db_lower_it = _db.lower_bound(*parent_arg_it);
							db_upper_it = _db.upper_bound(*parent_arg_it);
						}
						// adds task for next arg of parent, entry is a child in this context
						_call_stack.emplace(std::move(parent_arg_it), std::move(db_lower_it), std::move(db_upper_it), std::move(new_bindings), entry);
					}
					break;
				}
				case SOLVING_ROLLBACK:
				{
					// Removes last found_rule decision (that didn't unify)
					_call_stack.pop();
					if (_call_stack.empty()) {
						_solving_status = NO_SOLUTION;
						return false;
					}
					call_stack_entry& entry = _call_stack.top();
					if (_debug && entry.arg_it != entry.arg_it_cend()) {
						composed_term<T> term = entry.arg_it->deep_copy(*entry.orig_var_changelog_ptr);
						entry.orig_bindings.bind_results(term);
						print_spaces();
						std::cout << "rollback to " << term << std::endl;
					}
					// If I have more possibilities (in actual term)
					if (entry.arg_it != entry.arg_it_cend() && entry.found_rule_it != entry.found_rule_it_cend) {
						++entry.found_rule_it;
						_solving_status = SOLVING_FORWARD;
						// I will try it and we will see
					}

					break;
				}
				default:
					assert(false);
					exit(10);
				}
			}
			_solving_status = NO_SOLUTION;
			return false;
		}

		// Reruns solution of last resolving (calling `next()`)
		result_bindings<T>& solution() {
			assert(_solving_status == SOLVED);
			return _results;
		}

		// Printing method
		friend std::ostream& operator<< (std::ostream& stream, const solver<T>& sol) {
			assert(sol._solving_status == SOLVED);
			for (const auto& it : sol._results)
			{
				if (it.first.name() != "_") {
					stream << it.first.name();
					stream << " -> ";
					if (it.second.second.is_default_construed())
						stream << it.second.second;
					else if(it.second.first != "_")
						stream << it.second.first;
					else
						stream << '_' << std::hash<variable_model<T>>{}(it.second.first);

					stream << std::endl;
				}
			}
			return stream;
		}

	private:
		// Prints spaces based on depth of _call_stack
		void print_spaces() const {
			for (int i = 0; i < _call_stack.size(); ++i)
				std::cout << ' ';
		}
	};

}

#endif