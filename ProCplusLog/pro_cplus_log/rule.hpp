// Prolog rules and fact - used in database (rules without body is fact)

#ifndef PRO_CPLUS_LOG_RULE_H
#define PRO_CPLUS_LOG_RULE_H

#include "base.hpp"
#include "proxy_iterator.hpp"
#include<assert.h>
#include<string>
#include<memory>
#include<iostream>
#include<vector>

namespace prolog {

	// Prolog rule
	// eg. foo(bar, X) :- bar(X), dummy(bar).
	template<typename T = default_value_type> class rule {
		// Friend classes
		template<typename T> friend class database;
		template<typename A, class B> friend class proxy_const_iterator;

		using rule_type = rule<T>;
		using term_model_ptr = typename term<T>::model_ptr;

		using inner_iterator = typename std::vector<term_model_ptr>::iterator;
		using inner_const_iterator = typename std::vector<term_model_ptr>::const_iterator;

		using old_new_varibles_map = typename term_model<T>::old_new_varibles_map;

		std::shared_ptr<composed_term_model<T>> _head;
		std::vector<term_model_ptr> _body;
		old_new_varibles_map _old_new_var;
	public:
		// Typenames
		using signature_t = typename composed_term_model<T>::signature_t;

		using value_type = composed_term<T>;
		using reference = typename value_type&;
		using const_reference = const value_type&;
		using pointer = typename value_type*;
		using const_pointer = const value_type*;

		struct translator_type {
			inline static value_type convert(term_model_ptr& model) {
				return value_type(model);
			}

			inline static value_type shallow_copy(const value_type& val) {
				return val.shallow_copy();
			}
		};

		using iterator = proxy_iterator<rule_type, translator_type>;
		using const_iterator = proxy_const_iterator<rule_type, translator_type>;

		// Default constructor
		rule() {};
		// Creates new rule with specified head composed term
		rule(composed_term<T>&& head) {
			assert(head._model_ptr->type() == term_type::composed_type);
			_head = std::static_pointer_cast<composed_term_model<T>>(head._model_ptr);
		};
		// Copy rule (underlain terms are deep copied)
		rule(const rule<T>& orig) {
			_old_new_var = old_new_varibles_map();
			_head = orig._head->copy_model(_old_new_var);
			for (const auto& el : orig._body) {
				_body.push_back(el->copy(_old_new_var));
			}
		}

		const composed_term<T> head() const {
			return _head;
		}

		// Gives rule signature
		// eg. "foo(bar, X) :- bar(X), dummy(bar)." -> std::make_pair("foo",2)
		inline signature_t signature() {
			return _head->signature();
		}

		// Adds term to end of the rule
		void add_term(composed_term<T>&& t) {
			assert(t.type() == term_type::composed_type);
			_body.push_back(std::move(t._model_ptr));
		}

		// Printing method
		friend std::ostream& operator<< (std::ostream& stream, const rule<T>& res) {
			res._head->code_string(stream);

			if (res._body.size() > 0) {
				stream << " :- ";
				bool first = true;
				for (const auto& el : res._body)
				{
					if (first) {
						first = false;
					}
					else {
						stream << ", ";
					}
					el->code_string(stream);
				}
			}
			stream << "." << std::endl;
			return stream;
		}

		// In contrast of copy constructor makes shallow copy of object
		// The unification between shallow copies doesn't work well in every case
		rule shallow_copy() const {
			return term(_model_ptr);
		}

		// Iterators
		iterator begin() {
			return iterator(_body.begin());
		}

		iterator end() {
			return iterator(_body.end());
		}

		const_iterator cbegin() const {
			return const_iterator(const_cast<rule_type*>(this)->_body.begin());
		}

		const_iterator cend() const {
			return const_iterator(const_cast<rule_type*>(this)->_body.end());
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