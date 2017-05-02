// Database - object for storing and searching rules

#ifndef PRO_CPLUS_LOG_DATABASE_H
#define PRO_CPLUS_LOG_DATABASE_H

#include "rule.hpp"
#include<assert.h>
#include<string>
#include<memory>
#include<iostream>
#include<map>
#include<functional>

namespace prolog {

	// Const iterator
	template<typename _DB_T >
	class database_const_iterator : public std::iterator<std::bidirectional_iterator_tag, typename _DB_T::value_type>
	{
		// Friend classes
		template<typename T> friend class database;
	public:
		using value_type = typename _DB_T::value_type;
		using const_reference = typename _DB_T::const_reference;
		using const_pointer = typename _DB_T::const_pointer;

		// Own names
		using const_iterator = typename _DB_T::const_iterator;
		using inner_iterator = typename _DB_T::inner_iterator;
		using rule_ptr = typename _DB_T::rule_ptr;

		// Constructors
		database_const_iterator() : _inner_it() {}
		database_const_iterator(const const_iterator &it) : _inner_it(it._inner_it) {}
		database_const_iterator(const_iterator &&it) : _inner_it(std::move(it._inner_it)) {}

		// Assignment operators
		const_iterator& operator=(const const_iterator& x) {
			_inner_it = x._inner_it;
			return *this;
		}

		const_iterator& operator=(const_iterator&& x) {
			_inner_it = std::move(x._inner_it);
			return *this;
		}

		// Test operators
		bool operator==(const_iterator const& rhs) const
		{
			return (_inner_it == rhs._inner_it);
		}

		bool operator!=(const_iterator const& rhs) const
		{
			return (_inner_it != rhs._inner_it);
		}

		// Moving semantics
		const_iterator& operator++()
		{
			++_inner_it;
			return *this;
		}

		const_iterator operator++(int)
		{
			const_iterator result(*this);   // make a copy for result
			++_inner_it;					// Now use the prefix version to do the work
			return result;
		}

		const_iterator& operator--()
		{
			deinit();
			--_inner_it;
			return *this;
		}

		const_iterator operator--(int)
		{
			const_iterator result(*this);   // make a copy for result
			deinit();
			--_inner_it;					// Now use the prefix version to do the work
			return result;
		}

		//Getting values
		const_reference operator* () const
		{
			return *(_inner_it->second);
		}

		const_pointer operator->() const
		{
			return _inner_it->second.get();
		}

		// Returns original smart pointer that takes care of data
		const rule_ptr& data_owner() const {
			return const_cast<iterator*>(this)->_inner_it->second;
		}

	protected:
		inner_iterator	_inner_it;			//Iterator over keys		

		// Internal constructor
		database_const_iterator(inner_iterator&& it) : _inner_it(std::move(it)) {}
	};

	// Iterator
	template<typename _DB_T >
	class database_iterator : public database_const_iterator<_DB_T>
	{
		// Friend classes
		template<typename T> friend class database;
	public:
		using reference = typename _DB_T::reference;
		using pointer = typename _DB_T::pointer;
		using iterator = typename _DB_T::iterator;

		// Constructors
		database_iterator() : const_iterator() {}
		database_iterator(const iterator& it) : const_iterator(it) {};
		database_iterator(iterator&& it) : const_iterator(std::move(it)) {}

		// Assignment operators
		iterator& operator=(const iterator& x) {
			_inner_it = x._inner_it;
			return *this;
		}

		iterator& operator=(iterator&& x) {
			_inner_it = std::move(x._inner_it);
			return *this;
		}

		//Test operators 
		bool operator==(iterator const& rhs) const
		{
			return (_inner_it == rhs._inner_it);
		}

		bool operator!=(iterator const& rhs) const
		{
			return (_inner_it != rhs._inner_it);
		}

		//Move operators
		iterator& operator++()
		{
			++_inner_it;
			return *this;
		}

		iterator operator++(int)
		{
			iterator result(*this);
			deinit();
			++_inner_it;
			return result;
		}

		iterator& operator--()
		{
			--_inner_it;
			return *this;
		}

		iterator operator--(int)
		{
			iterator result(*this);
			--_inner_it;
			return result;
		}

		// Getting values
		reference operator*() const
		{
			return *(const_cast<iterator*>(this)->_inner_it->second);
		}

		pointer operator->() const
		{
			// Breaks const protection
			return const_cast<iterator*>(this)->_inner_it->second.get();
		}

		// Returns original smart pointer that takes care of data
		const rule_ptr& data_owner() const {
			return const_cast<iterator*>(this)->_inner_it->second;
		}
	protected:
		database_iterator(inner_iterator&& it) : const_iterator(std::move(it)) {}
	};

	// Database of rules
	template<typename T = default_value_type> class database {
		template<typename A> friend class database_const_iterator;
		// Private typenames
		using database_type = database<T>;

		using rule_ptr = std::unique_ptr<rule<T>>;
		using rule_signature_t = typename rule<T>::signature_t;
		using rules_multimap = std::multimap<rule_signature_t, rule_ptr>;

		using inner_iterator = typename rules_multimap::iterator;
		using inner_const_iterator = typename rules_multimap::const_iterator;

		// Variables
		rules_multimap _rules;
	public:
		// Typenames
		using value_type = rule<T>;
		using reference = typename value_type&;
		using const_reference = const value_type&;
		using pointer = typename value_type*;
		using const_pointer = const value_type*;

		using iterator = database_iterator<database_type>;
		using const_iterator = database_const_iterator<database_type>;

		// Adds new rule right after last rule with same signature
		iterator add_rule(rule<T>&& r) {
			rule_signature_t sig = r._head->signature();
			return _rules.insert(
				std::make_pair<rule_signature_t, rule_ptr>(
					std::move(sig),
					std::make_unique<rule<T>>(std::move(r))
					)
			);
		}

		// Adds new rule right after last rule with same signature
		iterator add_rule(std::unique_ptr<rule<T>>&& r) {
			rule_signature_t sig = r->_head->signature();
			return iterator(_rules.insert(
				std::make_pair<rule_signature_t, rule_ptr>(
					std::move(sig),
					std::move(r)
					))
			);
		}

		// Removes rule at iterator position
		void remove_rule(iterator& it) {
			_rules.erase(it);
		}

		// Searching methods 
		const_iterator cbegin() const {
			return _rules.cbegin();
		}

		const_iterator cend() const {
			return _rules.cend();
		}

		iterator begin() {
			return _rules.begin();
		}

		iterator end() {
			return _rules.end();
		}

		const_iterator begin() const {
			return cbegin();
		}

		const_iterator end() const {
			return cend();
		}

		iterator lower_bound(const composed_term<T>& t) {
			assert(t.type() == term_type::composed_type);
			return iterator(_rules.lower_bound(static_cast<composed_term_model<T>*>(t._model_ptr.get())->signature()));
		}

		iterator upper_bound(const composed_term<T>& t) {
			assert(t.type() == term_type::composed_type);
			return iterator(_rules.upper_bound(static_cast<composed_term_model<T>*>(t._model_ptr.get())->signature()));
		}

		const_iterator lower_bound(const composed_term<T>& t) const {
			assert(t.type() == term_type::composed_type);
			return const_iterator(const_cast<database*>(this)->_rules.lower_bound(static_cast<composed_term_model<T>*>(t._model_ptr.get())->signature()));
		}

		const_iterator upper_bound(const composed_term<T>& t) const {
			assert(t.type() == term_type::composed_type);
			return const_iterator(const_cast<database*>(this)->_rules.upper_bound(static_cast<composed_term_model<T>*>(t._model_ptr.get())->signature()));
		}

		// Printing method
		friend std::ostream& operator<< (std::ostream& stream, const database<T>& res) {
			for (const auto& sig_rule_ptr_pair : res._rules) {
				stream << *sig_rule_ptr_pair.second;
			}
			return stream;
		}
	};

}

#endif
