// Contains unify methods and result bindings

#ifndef PRO_CPLUS_LOG_TOOLS_H
#define PRO_CPLUS_LOG_TOOLS_H

// To disable debugging uncomment next line
// #define NDEBUG

#include "base.hpp"
#include <unordered_map>
#include <unordered_set>

namespace prolog {

	// ********************************************** Unify **********************************************

	// Unify two terms together and saves unification into result_bindings (previous result_bindings is respected)
	template<typename T = default_value_type> bool unify(const term<T>& t1, const term<T>& t2, result_bindings<T>& bindings) {
		using term_model_ptr = typename term<T>::model_ptr;
		using arg_ptr_vector = typename result_bindings<T>::arg_ptr_vector;
		
		std::stack<term_model_ptr> term_stack_left;
		std::stack<term_model_ptr> term_stack_right;

		term_stack_left.push(t1._model_ptr);
		term_stack_right.push(t2._model_ptr);

		// For each arg
		while (term_stack_left.size() > 0)
		{
			term_model_ptr l = std::move(term_stack_left.top());
			term_model_ptr r = std::move(term_stack_right.top());
			term_stack_left.pop();
			term_stack_right.pop();

			// If booth args has value
			if (bindings.has_value(*l) && bindings.has_value(*r)) {
				// Do they have same functors?
				if (bindings.functor(*l) != bindings.functor(*r))
					return false;

				const arg_ptr_vector& args_l = bindings.args(*l);
				const arg_ptr_vector& args_r = bindings.args(*r);

				if (args_l.size() != args_r.size()) // Has terms different arguments count?
					return false;

				// I will check their args
				for (auto it = args_l.crbegin(); it != args_l.crend(); ++it) {
					term_stack_left.push(*it);
				}

				for (auto it = args_r.crbegin(); it != args_r.crend(); ++it) {
					term_stack_right.push(*it);
				}
			}
			else { // l or p is non-binded variable
				switch (l->type())
				{
				case term_type::composed_type:
					bindings.bind_variable(*(static_cast<variable_model<T>*>(r.get())), l);
					break;
				case term_type::variable_type:
					switch (r->type())
					{
					case term_type::composed_type:
						bindings.bind_variable(*(static_cast<variable_model<T>*>(l.get())), r);
						break;
					case term_type::variable_type:
						bindings.bind_variables(*(static_cast<variable_model<T>*>(l.get())), *(static_cast<variable_model<T>*>(r.get())));
						break;
					default:
						assert(false);
						std::cerr << "Unknown term type of l" << std::endl;
						exit(1);
					}
					break;
				default:
					assert(false);
					std::cerr << "Unknown term type of p" << std::endl;
					exit(1);
				}
			}

		}

		return true;
	}

	// ********************************************** Binding **********************************************

	// Results of unification
	// eg. X -> foo(bar), Y -> bar, Z -> Q
	template<typename T = default_value_type> class result_bindings {
		// Friend classes
		template<typename A, class B> friend class proxy_const_iterator;

		using result_bindings_type = result_bindings<T>;
		using old_new_varibles_map = typename term<T>::old_new_varibles_map;

		// Helping structure for saving results of variable unification (more variables could be unified together)
		struct bind_model {
			std::shared_ptr<composed_term_model<T>> binded; // Actual unified value
			std::size_t last_change_id = 1;					// Internal helping number
			std::unordered_set<variable_model<T>> binded_variables; // Set of variables unified to this value

			bind_model(const variable_model<T>& model) {
				binded_variables.insert(model);
			};

			// object are same it they are in same place in memory
			bool operator ==(const bind_model &b) const {
				return this == &b;
			}

			// object are same it they are in same place in memory
			bool operator !=(const bind_model &b) const {
				return !operator==(b);
			}
		};

		using bindings_table = std::unordered_map<variable_model<T>, std::shared_ptr<bind_model>>;

		bindings_table _bindings;
		std::size_t _last_change_id = 1; // Helping number for marking last change on bind_model

		using inner_iterator = typename bindings_table::iterator;
		using inner_const_iterator = typename bindings_table::const_iterator;
	public:
		using value_type = std::pair<variable_term<T>, std::pair<T, term<T>>>;
		using reference = typename value_type&;
		using const_reference = const value_type&;
		using pointer = typename value_type*;
		using const_pointer = const value_type*;

		struct translator_type {
			inline static value_type convert(std::pair<const variable_model<T>, std::shared_ptr<bind_model>>& entry) {
				return std::make_pair(variable_term<T>(entry.first), 
					std::make_pair(entry.second->binded_variables.begin()->name(), term<T>(entry.second->binded)));
			}

			inline static value_type shallow_copy(const value_type& val) {
				return std::make_pair(val.first.shallow_copy(), val.second.shallow_copy());
			}
		};

		using iterator = proxy_iterator<result_bindings_type, translator_type>;
		using const_iterator = proxy_const_iterator<result_bindings_type, translator_type>;

		using arg_ptr_vector = typename composed_term_model<T>::term_model_ptr_vector;
		using term_model_ptr = typename term<T>::model_ptr;

		// Default constructor
		result_bindings() {};
		// Copy constructor (does NOT change anything, but puts bind_models to new place in memory, so they are undefended from source)
		result_bindings(const result_bindings& bindings) : _last_change_id(bindings._last_change_id) {
			
			for (auto it = bindings._bindings.cbegin(); it != bindings._bindings.cend(); ++it) {
				if (!_bindings[it->first]) {
					_bindings[it->first] = std::make_shared<bind_model>(*it->second);
					for (const variable_model<T>& model : it->second->binded_variables)
						_bindings[model] = _bindings[it->first];
				}
			}
		}

		// Test if term has some unified or original value 
		bool has_value(const term_model<T>& t) const {
			switch (t.type())
			{
			case term_type::composed_type:
				return true;
			case term_type::variable_type: {
				// Has already unified value?
				const auto& it = _bindings.find(static_cast<const variable_model<T>&>(t));
				if (it != _bindings.end()) {
					// Is variable unified to variable?
					if (it->second->binded)
						return true;
					else
						return false;
				}
				else
					return false;
			}
			default:
				assert(false);
				std::cerr << "Unknown term type" << std::endl;
				exit(1);
			}
		}

		// Returns original or unified args of term_model
		const arg_ptr_vector& args(const term_model<T>& t) const {
			assert(has_value(t));

			switch (t.type())
			{
			case term_type::composed_type:
				return static_cast<const composed_term_model<T>&>(t)._args;
			case term_type::variable_type: {
				return const_cast<result_bindings*>(this)->
					_bindings[static_cast<const variable_model<T>&>(t)]
					->binded->_args;
			}
			default:
				assert(false);
				std::cerr << "Unknown term type" << std::endl;
				exit(1);
			}
		}

		// Returns original or unified functor of term_model
		const T& functor(const term_model<T>& t) const {
			assert(has_value(t));

			switch (t.type())
			{
			case term_type::composed_type:
				return static_cast<const composed_term_model<T>&>(t)._functor;
			case term_type::variable_type: {
				return const_cast<result_bindings*>(this)->
					_bindings[static_cast<const variable_model<T>&>(t)]
					->binded->_functor;
			}
			default:
				assert(false);
				std::cerr << "Unknown term type" << std::endl;
				exit(1);
			}
		}

		// Unify variable with term_model together
		void bind_variable(const variable_model<T>& var, const term_model_ptr& comp_term_ptr) {
			assert(var.type() == term_type::variable_type);
			assert(comp_term_ptr->type() == term_type::composed_type);
			// Is variable binded to another variable?
			if (!_bindings[var]) {
				_bindings[var] = std::make_shared<bind_model>(var);
			}
			assert(!_bindings[var]->binded); // Hasn't any binded value now
			_bindings[var]->binded = std::static_pointer_cast<composed_term_model<T>>(comp_term_ptr);
			_bindings[var]->binded_variables.insert(var);
		}

		// Bind two variables together
		void bind_variables(const variable_model<T>& var1, const variable_model<T>& var2) {
			assert(var1.type() == term_type::variable_type);
			assert(var2.type() == term_type::variable_type);

			if (_bindings[var1] && _bindings[var2]) {
				// both variables are binded to something
				
				// This should be solved by unify function
				assert(!_bindings[var1]->binded && !_bindings[var2]->binded);
				// values should be binded to bindings model without value

				if (_bindings[var1] == _bindings[var2]) 
					return; // They are binded together already
				
				// I have to connect bind_models together
				std::shared_ptr<bind_model> model = _bindings[var2];
				for (auto it = model->binded_variables.cbegin(); it!= model->binded_variables.cend(); ++it) {
					_bindings[*it] = _bindings[var1];
					_bindings[var1]->binded_variables.insert(*it);
				}
			}
			else {
				// one of variables is not binded to something
				if (!_bindings[var1]) {
					_bindings[var1] = std::make_shared<bind_model>(var1);
				}
				if (!_bindings[var2]) {
					_bindings[var2] = std::make_shared<bind_model>(var2);
				}

				if (_bindings[var1]->binded) {
					_bindings[var2] = _bindings[var1];
					_bindings[var1]->binded_variables.insert(var2);
				}
				else {
					_bindings[var1] = _bindings[var2];
					_bindings[var2]->binded_variables.insert(var1);
				}
			}
		}

		// Clear results of unification 
		void reset() {
			_bindings = bindings_table();
		}

		// Replaces variables in term with known unified values (unknown variables didn't change)
		// eg foo(X), X -> bar  ---->  foo(bar)
		void bind_results(term_model_ptr& model_ptr) const {
			std::stack<term_model_ptr> term_stack;
			// If term is variable it will be easier
			if (model_ptr->type() == term_type::variable_type) {
				// Find result of unification
				const auto& el = _bindings.find(*static_cast<variable_model<T>*>(model_ptr.get()));
				if (el != _bindings.end() && el->second->binded) { // If binded
					assert(el->second->binded->_type == term_type::composed_type);
					// Replace value
					model_ptr = el->second->binded;
					// Continue with replacing inside
					term_stack.push(model_ptr);
				}
			}
			else
				term_stack.push(model_ptr);

			// For each arg in term
			while (term_stack.size() > 0)
			{
				term_model_ptr model_ptr = std::move(term_stack.top());
				term_stack.pop();

				assert(model_ptr->type() == term_type::composed_type);

				arg_ptr_vector& args
					= static_cast<composed_term_model<T>*>(model_ptr.get())->_args;

				// For each arg on actual level
				for (auto it = args.rbegin(); it != args.rend(); ++it) {
					if ((*it)->type() == term_type::composed_type)
						term_stack.push(*it);
					else {
						assert((*it)->type() == term_type::variable_type); // Variable
						// Find result of unification for actual variable
						const auto& bounded_el = _bindings.find(*static_cast<variable_model<T>*>(it->get()));
						if (bounded_el != _bindings.end() && bounded_el->second->binded) { // If binded
							assert(bounded_el->second->binded->type() == term_type::composed_type);
							// Replace value
							*it = bounded_el->second->binded;
							term_stack.push(*it);
							// Continue with replacing inside
						}
						// If not binded I don't need to care
					}
				}
			}
		}

		// Replaces variables in term with known unified values (unknown variables didn't change)
		// eg foo(X), X -> bar  ---->  foo(bar)
		void bind_results(term<T>& term) const {
			bind_results(term._model_ptr);
		}

		// Replaces variables in binded values with known unified values (unknown variables didn't change)
		// eg foo(X), X -> bar  ---->  foo(bar)
		void bind_inside() {
			for (auto& element: _bindings)
				if(element.second->binded)
					bind_results(std::static_pointer_cast<term_model<T>>(element.second->binded));
		}

		// Adds variables from "from" result_bindings that are in variables to this result bindings
		// "variables" is usual map of variables in one rule (so I can merge only important variables)
		void merge(const result_bindings<T>& from, const old_new_varibles_map& varibles) {
			std::size_t last_change_id = 0;
			++_last_change_id;
			// For each variable
			std::unordered_set<variable_model<T>> vars_hs;
			for (auto& pairs : varibles)
				vars_hs.insert(*pairs.second);
			for (auto& var : vars_hs)
			{
				if (!has_value(var)) {
					const auto& found = from._bindings.find(var);
					if (found != from._bindings.end()) {
						bind_model& found_model = *found->second;
						if (found_model.binded) {
							// var has in source result_bindings some value
							term_model_ptr model_ptr = found_model.binded->clone();
							from.bind_results(std::static_pointer_cast<term_model<T>>(model_ptr));
							bind_variable(var, model_ptr);
						}
						else {
							// var hasn't in source result_bindings any value but is binded to another value
							const bind_model& found_model = *found->second;
							if (found_model.last_change_id == _last_change_id) // I already update it
								continue;
							// for all variables binded together
							for (const variable_model<T>& source : found_model.binded_variables) {
								// if variable is from my list of variables (from variables I care)
								if (vars_hs.find(source) != vars_hs.end() && var != source) {
									// I bind them together
									bind_variables(var, source);
								}
							}
							const_cast<bind_model&>(found_model).last_change_id = _last_change_id;
						}
					}
				}
			}
		}

		// Unify unknown variables with known variables in "from" result bindings
		void merge(const result_bindings<T>& from) {
			for (auto& it : _bindings)
			{
				if (!it.second->binded) {
					const auto& found = from._bindings.find(it.first);
					if (found != from._bindings.end()) {
						it.second->binded = found->second->binded;
					}
				}
			}
		}

		// Test if term has some unified value
		bool has_value(const term<T>& t) const {
			return has_value(*t._model_ptr);
		}

		// Returns result of unification for term (if has some unified value)
		composed_term<T> operator[](const term<T>& t) {
			assert(t.type() == term_type::variable_type);
			assert(has_value(t));
			return composed_term<T>(
				_bindings[*static_cast<variable_model<T>*>(t._model_ptr.get())]->binded
				);
		}

		// Printing method
		friend std::ostream& operator<< (std::ostream& stream, const result_bindings<T>& res) {
			for (const auto& it : res._bindings)
			{
				stream << it.first.name();
				stream << " -> ";
				if (it.second->binded)
					it.second->binded->code_string(stream);
				else
					stream << '_' << it.second->binded_variables.cbegin()->name();

				stream << std::endl;
			}
			return stream;
		}

		// Iterators
		const_iterator cbegin() const {
			return const_iterator(const_cast<result_bindings<T>*>(this)->_bindings.begin());
		}

		const_iterator cend() const {
			return const_iterator(const_cast<result_bindings<T>*>(this)->_bindings.end());
		}

		iterator begin() {
			return iterator(_bindings.begin());
		}

		iterator end() {
			return iterator(_bindings.end());
		}

		const_iterator begin() const {
			return cbegin();
		}

		const_iterator end() const {
			return cend();
		}
		
	};
}

#endif
