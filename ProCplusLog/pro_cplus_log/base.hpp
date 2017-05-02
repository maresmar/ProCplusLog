// Basic Prolog objects: term, variables and composed terms

#ifndef PRO_CPLUS_LOG_BASE_H
#define PRO_CPLUS_LOG_BASE_H

#include<assert.h>
#include<string>
#include<vector>
#include<memory>
#include<iostream>
#include<stack>
#include<unordered_map>
#include "proxy_iterator.hpp"

namespace prolog {

	enum term_type { composed_type, variable_type };

	// Forward definitions
	template<typename T> class term_model;
	template<typename T> class composed_term_model;
	template<typename T> class variable_model;

	// Default type of functor and atoms (when is template unspecialized eg term<>)
	using default_value_type = std::string;

	// ********************************************** Objects ********************************************** 

	// Prolog term - base of everything
	// eg. peter, Variable, father(peter, emma)
	// Contains only shared pointer to data
	template<typename ValType = default_value_type> class term {
		// Friend classes / functions
		template<typename T> friend class composed_term;
		template<typename T> friend class variable_term;
		template<typename T> friend class result_bindings;
		template<typename T> friend bool unify(const term<T>& t1, const term<T>& t2, result_bindings<T>& bindings);
		template<typename T> friend class database;
		template<typename T> friend class rule;

		// Typenames
		using model_ptr = typename term_model<ValType>::model_ptr;

		model_ptr _model_ptr;
		// Model constructors
		term(const model_ptr& _ptr) : _model_ptr(_ptr) {};
		term(model_ptr&& _ptr) : _model_ptr(std::move(_ptr)) {};
	public:
		// Public typenames
		using old_new_varibles_map = typename term_model<ValType>::old_new_varibles_map;

		// Default constructor
		term()  : _model_ptr() {};
		// Deep copy constructor
		term(const term& t)  {
			if (t._model_ptr)
				_model_ptr = t._model_ptr->deep_copy();
		};
		// Move constructor
		term(term&& t) : _model_ptr(std::move(t._model_ptr)) {
		}

		// Assignment operators
		term& operator=(const term& x) = default;

		term& operator=(term&& x) = default;

		// Gives type of term eg. composed_type or variable_type
		inline const term_type type() const {
			return _model_ptr->type();
		}

		// In contrast of copy constructor makes shallow copy of object
		// The unification between shallow copies doesn't work well in every case
		term shallow_copy() const {
			return term(_model_ptr);
		}

		// Deep copy - same as copy constructor, but you log and save changes in variables
		term deep_copy(old_new_varibles_map& new_variables) const {
			return term(_model_ptr->copy(new_variables));
		}

		bool is_default_construed() const {
			if (_model_ptr)
				return true;
			else
				return false;
		}

		friend std::ostream& operator<< (std::ostream& stream, const term& t) {
			t._model_ptr->code_string(stream);
			return stream;
		}
	};

	// Composed term 
	// eg. father(peter, emma)
	// Does NOT contains any other data except some functions (can bee sliced)
	template<typename T = default_value_type> class composed_term : public term<T> {
		// Friend classes
		template<typename T> friend class result_bindings;
		template<typename T> friend class rule;
		template<typename A, class B> friend class proxy_const_iterator;

		using composed_term_type = composed_term<T>;

		using inner_iterator = typename composed_term_model<T>::iterator;
		using inner_const_iterator = typename composed_term_model<T>::const_iterator;

		// Model constructors
		composed_term(model_ptr&& ptr) : term<T>::term(std::move(ptr)) {};
		composed_term(const model_ptr& ptr) : term<T>::term(ptr) {};
	public:
		using value_type = term<T>;
		using reference = typename value_type&;
		using const_reference = const value_type&;
		using pointer = typename value_type*;
		using const_pointer = const value_type*;

		struct translator_type {
			inline static value_type convert(model_ptr& model) {
				return value_type(model);
			}

			inline static value_type shallow_copy(const value_type& val) {
				return val.shallow_copy();
			}
		};

		using iterator = proxy_iterator<composed_term_type, translator_type>;
		using const_iterator = proxy_const_iterator<composed_term_type, translator_type>;

		// Default constructor
		composed_term() : term<T>::term() {};
		// Crate new term with specific functor
		composed_term(const T& functor)
			: composed_term(std::make_shared<composed_term_model<T>>(functor)) {};
		// Crate new term with specific functor and args
		composed_term(const T& functor, std::vector<term<T>>&& args)
			: composed_term(std::make_shared<composed_term_model<T>>(functor, std::move(args))) {};

		// In contrast of copy constructor makes shallow copy of object
		// The unification between shallow copies doesn't work well in every case
		composed_term shallow_copy() const {
			return composed_term(_model_ptr);
		}

		// Deep copy - same as copy constructor, but you log and save changes in variables
		composed_term deep_copy(old_new_varibles_map& new_variables) const {
			return composed_term(_model_ptr->copy(new_variables));
		}

		void add_arg(const term<T>& t) {
			assert(term<T>::type() == term_type::composed_type);
			static_cast<composed_term_model<T>*>(term<T>::_model_ptr.get())->add_arg(t._model_ptr);
		}

		void add_arg(term<T>&& t) {
			assert(term<T>::type() == term_type::composed_type);
			static_cast<composed_term_model<T>*>(term<T>::_model_ptr.get())->add_arg(std::move(t._model_ptr));
		}

		const T functor() const {
			return static_cast<composed_term_model<T>*>(_model_ptr.get())->_functor;
		}

		// Iterators
		const_iterator cbegin() const {
			return const_iterator(static_cast<composed_term_model<T>*>(term<T>::_model_ptr.get())->begin());
		}

		const_iterator cend() const {
			return const_iterator(static_cast<composed_term_model<T>*>(term<T>::_model_ptr.get())->end());
		}

		iterator begin() {
			return iterator(static_cast<composed_term_model<T>*>(term<T>::_model_ptr.get())->begin());
		}

		iterator end() {
			return iterator(static_cast<composed_term_model<T>*>(term<T>::_model_ptr.get())->end());
		}

		const_iterator begin() const {
			return cbegin();
		}

		const_iterator end() const {
			return cend();
		}
	};

	// Variable term 
	// eg. "Variable" or "_"
	// Does NOT contains any other data except some functions (can bee sliced)
	template<typename T = default_value_type> class variable_term : public term<T> {
		// Friend classes
		template<typename T> friend class result_bindings;

		// Model constructors
		variable_term(model_ptr&& ptr) : term(std::move(ptr)) {};
		variable_term(const model_ptr& ptr) : term(ptr) {};
		// Model copy constructor (only used in iterators over varible_terms_model containers)
		variable_term(const variable_model<T>& model) : term(std::make_shared<variable_model<T>>(model)) {};
	public:
		// Default constructor
		variable_term() : term<T>::term() {};
		// Create new varible_term with specific name
		variable_term(const T& name)
			: variable_term(std::make_shared<variable_model<T>>(name)) {};

		// In contrast of copy constructor makes shallow copy of object
		// The unification between shallow copies doesn't work well in every case
		variable_term shallow_copy() const {
			return variable_term(_model_ptr);
		}

		// Deep copy - same as copy constructor, but you log and save changes in variables
		variable_term deep_copy(old_new_varibles_map& new_variables) const {
			return variable_term(_model_ptr->copy(new_variables));
		}

		const T name() const {
			return static_cast<variable_model<T>*>(_model_ptr.get())->name();
		}
	};

	// ********************************************** Models ********************************************** 

	// Term data model
	template<typename T>class term_model {
	protected:
		const term_type _type;
		term_model(term_type type) : _type(type) {}
	public:
		// Shared pointer to data (shared because of unification)
		using model_ptr = std::shared_ptr<term_model>;
		// Stores connection between old and new variable in copy
		using old_new_varibles_map = std::unordered_map<variable_model<T>, std::shared_ptr<variable_model<T>>>;

		inline const term_type type() const {
			return _type;
		}

		// Deep copy constructor
		virtual model_ptr copy(old_new_varibles_map& new_variables) const = 0;
		model_ptr deep_copy()  const {
			return copy(old_new_varibles_map());
		}
		// Clones the object to new place in memory
		virtual model_ptr clone() const = 0;

		// Printing methods
		virtual void code_string(std::ostream& ss) const = 0;
		virtual bool has_value() const = 0;

		// Destructor
		virtual ~term_model() {};
	};

	template<typename T> class composed_term_model : public term_model<T> {
		// Friend classes
		template<typename Y> friend class result_bindings;
		template<typename Y> friend class composed_term;
	private:
		// Typenames
		using term_model_ptr_vector = std::vector<model_ptr>;

		T _functor; //if args.size() == 0 then it's atom
		term_model_ptr_vector _args;

		// Tests if term is list eg. [ xxx | yyy ]
		bool ilist() const {
			return _functor == "[";
		}
	public:
		using signature_t = std::pair<T, std::size_t>;
		using value_type = typename term_model_ptr_vector::value_type;
		using reference = typename term_model_ptr_vector::reference;
		using const_reference = typename term_model_ptr_vector::const_reference;
		using pointer = typename term_model_ptr_vector::pointer;
		using const_pointer = typename term_model_ptr_vector::const_pointer;

		using iterator = typename term_model_ptr_vector::iterator;
		using const_iterator = typename term_model_ptr_vector::const_iterator;

		// Creates new composed term with specified functor and args
		composed_term_model(const T& functor, term_model_ptr_vector&& args) : _functor(functor), _args(std::move(args)), term_model(term_type::composed_type) {};
		
		// Creates new composed term with specified functor
		composed_term_model(const T& functor) : composed_term_model(functor, term_model_ptr_vector()) {};

		// Deep copy method (with accurate return type)
		std::shared_ptr<composed_term_model> copy_model(old_new_varibles_map& new_variables) const {
			std::shared_ptr<composed_term_model> result = std::make_shared<composed_term_model>(_functor);
			std::stack <std::reference_wrapper<const term_model>> term_stack_orig;
			std::stack<std::reference_wrapper<term_model>> term_stack_copy;

			term_stack_orig.push(*this);
			term_stack_copy.push(*result);

			while (term_stack_orig.size() > 0)
			{
				const term_model& orig_model_ref = term_stack_orig.top();
				term_stack_orig.pop();
				assert(orig_model_ref.type() == term_type::composed_type);

				const term_model_ptr_vector& args_orig
					= static_cast<const composed_term_model<T>&>(orig_model_ref)._args;

				term_model& copy_model_ref = term_stack_copy.top();
				term_stack_copy.pop();

				term_model_ptr_vector& args_copy
					= static_cast<composed_term_model<T>&>(copy_model_ref)._args;

				// Copy args on actual level
				for (const auto& orig : args_orig) {
					if (orig->type() == term_type::composed_type) {
						composed_term_model& model = *static_cast<composed_term_model<T>*>(orig.get());
						args_copy.push_back(std::make_shared<composed_term_model<T>>(model._functor));
					}
					else {
						assert(orig->type() == term_type::variable_type);
						variable_model<T>& model = *static_cast<variable_model<T>*>(orig.get());
						if (!new_variables[model]) {
							new_variables[model] = std::make_shared<variable_model<T>>(model.name());
							assert(new_variables[model]->type() == term_type::variable_type);
						}
						args_copy.push_back(new_variables[model]);
					}
				}

				// Simulating calling stack
				auto it_copy = args_copy.rbegin();
				for (auto it_orig = args_orig.rbegin(); it_orig != args_orig.rend(); ++it_orig, ++it_copy) {
					if ((*it_orig)->type() == term_type::composed_type) {
						term_stack_orig.push(**it_orig);
						term_stack_copy.push(**it_copy);
					}
					// If not composed I don't need to care
				}
				assert(it_copy == args_copy.rend());
			}

			return std::move(result);
		}

		// Deep copy method
		virtual model_ptr copy(old_new_varibles_map& new_variables) const {
			return std::static_pointer_cast<term_model<T>>(copy_model(new_variables));
		}

		virtual model_ptr clone() const {
			std::shared_ptr<composed_term_model> result = std::make_shared<composed_term_model>(_functor);
			std::stack <std::reference_wrapper<const term_model>> term_stack_orig;
			std::stack<std::reference_wrapper<term_model>> term_stack_copy;

			term_stack_orig.push(*this);
			term_stack_copy.push(*result);

			while (term_stack_orig.size() > 0)
			{
				const term_model& orig_model_ref = term_stack_orig.top();
				term_stack_orig.pop();
				assert(orig_model_ref.type() == term_type::composed_type);

				const term_model_ptr_vector& args_orig
					= static_cast<const composed_term_model<T>&>(orig_model_ref)._args;

				term_model& copy_model_ref = term_stack_copy.top();
				term_stack_copy.pop();

				term_model_ptr_vector& args_copy
					= static_cast<composed_term_model<T>&>(copy_model_ref)._args;

				// Copy args on actual level
				for (const auto& orig : args_orig) {
					if (orig->type() == term_type::composed_type) {
						composed_term_model& model = *static_cast<composed_term_model<T>*>(orig.get());
						args_copy.push_back(std::make_shared<composed_term_model<T>>(model._functor));
					}
					else {
						assert(orig->type() == term_type::variable_type);
						variable_model<T>& model = *static_cast<variable_model<T>*>(orig.get());
						args_copy.push_back(std::make_shared<variable_model<T>>(model));
					}
				}

				// Simulating calling stack
				auto it_copy = args_copy.rbegin();
				for (auto it_orig = args_orig.rbegin(); it_orig != args_orig.rend(); ++it_orig, ++it_copy) {
					if ((*it_orig)->type() == term_type::composed_type) {
						term_stack_orig.push(**it_orig);
						term_stack_copy.push(**it_copy);
					}
					// If not composed I don't need to care
				}
				assert(it_copy == args_copy.rend());
			}

			return std::move(result);
		}

		// Printing methods
		virtual void code_string(std::ostream& ss) const {
			ss << _functor;
			if (_args.size() > 0) {
				if(!ilist())
					ss << '(';
				bool first = true;
				for (const model_ptr& arg : _args) {
					if (first) {
						first = false;
					}
					else {
						if (!ilist())
							ss << ',';
						else
							ss << '|';
					}
					arg->code_string(ss);
				}
				if (!ilist())
					ss << ')';
				else
					ss << ']';
			} else if(ilist())
				ss << ']';
		}

		void add_arg(model_ptr&& t) {
			_args.push_back(std::move(t));
		}

		// For variables - variable could be on multiple places
		void add_arg(const model_ptr& t) {
			_args.push_back(std::move(t));
		}

		virtual bool has_value() const {
			return true;
		}

		// Returns signature of composed term eg. std::make_pair<"foo",3> for "foo(X, bar, Y)"
		// Useful for rules
		inline signature_t signature() const {
			return std::make_pair(_functor, _args.size());
		}

		// Iterators
		iterator begin() {
			return _args.begin();
		}

		iterator end() {
			return _args.end();
		}

		const_iterator cbegin() const {
			return _args.cbegin();
		}

		const_iterator cend() const {
			return _args.cend();
		}
	};

	template<typename T> class variable_model : public term_model<T> {
		template<typename Y> friend class variable_term;
		template<typename Y> friend class result_bindings;
		template<typename Y> friend struct std::hash;
	public:
		using hash_t = typename std::size_t;
	private:
		T _name;
		hash_t _hash; // Used for hash and comparing (variable_model must be copy constructible (because of std containers)
					  // but I need to compare them based on their address in memory (or something similar)
	public:
		// Create new variable model with specific name
		variable_model(const T& name) : _name(name), _hash((std::size_t)this), term_model(term_type::variable_type) {}

		inline const T& name() const {
			return _name;
		}

		virtual bool has_value() const {
			return false;
		}

		// Deep copy method (with accurate return type)
		std::shared_ptr<variable_model> copy_model(old_new_varibles_map& new_variables) const {
			if (!new_variables[*this]) {
				new_variables[*this] = std::make_shared<variable_model>(this->_name);
				assert(new_variables[*this]->type() == term_type::variable_type);
			}
			return new_variables[*this];
		}

		// Deep copy method
		virtual model_ptr copy(old_new_varibles_map& new_variables) const {
			return std::static_pointer_cast<term_model<T>>(copy_model(new_variables));
		}

		// Clones the object to new place in memory
		virtual model_ptr clone() const {
			return std::static_pointer_cast<term_model<T>>(std::make_shared<variable_model>(*this));
		}

		// Printing methods
		virtual void code_string(std::ostream& ss) const {
			ss << _name;
		}

		// Standard compare methods
		bool operator==(const variable_model& other) const
		{
			return _hash == other._hash;
		}

		bool operator!=(const variable_model& other) const
		{
			return !operator==(other);
		}
	};
}

// ********************************************** Hashing related methods **********************************************

namespace std
{
	template<typename T> struct hash<prolog::variable_model<T>>
	{
		using argument_type = prolog::variable_model<T>;
		using result_type = std::size_t;
		result_type operator()(const argument_type& var) const
		{
			return var._hash;
		}
	};
}

#endif
