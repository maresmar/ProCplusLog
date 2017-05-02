#ifndef PROXY_ITERATOR_H
#define PROXY_ITERATOR_H

// Transform one iterator to another (from inner iterator creates new object that is returned from proxy_iterator)
// eg. simplify code from `composed_term(*model_ptr_it)` to `*composed_term_it`

/*
// Example of _Translator (using value_type = composed_term<T>)
struct translator_type {
	inline static value_type convert(term_model_ptr& model) {
		return value_type(model);
	}

	inline static value_type shallow_copy(const value_type& val) {
		return val.shallow_copy();
	}
};
*/

// Const iterator
template<typename _CONTAINER_T, class _Translator> // _CONTAINER_T - Original container,  _Translator - transform methods 
class proxy_const_iterator : public std::iterator<std::bidirectional_iterator_tag, typename _CONTAINER_T::value_type>
{
public:
	using value_type = typename _CONTAINER_T::value_type;
	using const_reference = typename _CONTAINER_T::const_reference;
	using const_pointer = typename _CONTAINER_T::const_pointer;

	// Own names
	using const_iterator = typename _CONTAINER_T::const_iterator;
	using inner_iterator = typename _CONTAINER_T::inner_iterator;

	// Constructors
	proxy_const_iterator(inner_iterator&& it) : _inner_it(std::move(it)), _inited(false), _val() {}
	proxy_const_iterator() : _inner_it() {}
	proxy_const_iterator(const const_iterator &it) : _inner_it(it._inner_it), _inited(it._inited), _val(_Translator::shallow_copy(it._val)) {}
	proxy_const_iterator(const_iterator &&it) : _inner_it(std::move(it._inner_it)), _inited(it._inited), _val(std::move(it._val)) {}

	// Assignment operators
	const_iterator& operator=(const const_iterator& x) {
		_inner_it = x._inner_it;
		_inited = x._inited;
		_val = _Translator::shallow_copy(x._val);
		return *this;
	}

	const_iterator& operator=(const_iterator&& x) {
		_inner_it = std::move(x._inner_it);
		_inited = x._inited;
		_val = std::move(x._val);
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
		deinit();
		++_inner_it;
		return *this;
	}

	const_iterator operator++(int)
	{
		const_iterator result(*this);   // make a copy for result
		deinit();
		++_inner_it;					 // Now use the prefix version to do the work
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
	const value_type operator* () const
	{
		// Breaks const protection
		return _Translator::convert(*const_cast<const_iterator*>(this)->_inner_it);
		// Normally operator*() should return const_reference
		// But here is reference inside term (value_type) because of model_ptr (in term)
	}

	const_pointer operator->() const
	{
		//Breaks const protection
		const_cast<const_iterator*>(this)->init();
		return &_val;
	}

	//Destructor
	virtual ~proxy_const_iterator() {
		deinit();
	}

protected:
	bool			_inited = false;	//Tels if data are loaded from file
	value_type		_val;				//Shared pointer to value_type (std::pair<key, value>)
	inner_iterator	_inner_it;			//Iterator over keys		

										// Internal constructor


	// Create new wrapper object around model (I need to store it inside iterator because of pointer in operator->)
	void init() {
		if (_inited == false) {
			_val = _Translator::convert(*_inner_it);
			_inited = true;
		}
	}

	// Destroyed wrapper object around model
	void deinit() {
		_inited = false;
		_val = value_type();
	}
};

// Iterator
template<typename _CONTAINER_T, class _Translator >
class proxy_iterator : public proxy_const_iterator<_CONTAINER_T, _Translator>
{
public:
	using reference = typename _CONTAINER_T::reference;
	using pointer = typename _CONTAINER_T::pointer;
	using iterator = typename _CONTAINER_T::iterator;

	// Constructors
	proxy_iterator(inner_iterator&& it) : const_iterator(std::move(it)) {}
	proxy_iterator() : const_iterator() {}
	proxy_iterator(const iterator& it) : const_iterator(it) {};
	proxy_iterator(iterator&& it) : const_iterator(std::move(it)) {}

	// Assignment operators
	iterator& operator=(const iterator& x) {
		deinit();
		_inner_it = x._inner_it;
		_inited = x._inited;
		_val = _Translator::shallow_copy(x._val);
			return *this;
	}

	iterator& operator=(iterator&& x) {
		deinit();
		_inner_it = std::move(x._inner_it);
		_inited = x._inited;
		_val = std::move(x._val);
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
		deinit();
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
		deinit();
		--_inner_it;
		return *this;
	}

	iterator operator--(int)
	{
		iterator result(*this);
		deinit();
		--_inner_it;
		return result;
	}

	// Getting values
	value_type operator*() const
	{
		// Breaks const protection
		return _Translator::convert(*const_cast<iterator*>(this)->_inner_it);
		// Normally operator*() should return const_reference
		// But here is reference inside term (value_type) because of model_ptr (in term)
	}

	pointer operator->() const
	{
		// Breaks const protection
		const_cast<iterator*>(this)->init();
		return &const_cast<iterator*>(this)->_val;
	}
};

#endif // !PROXY_ITERATOR_H