// Test file - mostly for compiler
// not used in app 


#include <string>
#include <iostream>
#include "ProCplusLog.hpp"

int test_id = 1;

void test_unify(const prolog::term<>& t1, const prolog::term<>& t2) {
	std::cout << "====================" << std::endl;
	std::cout << "==== Test unify ====" << std::endl;
	std::cout << "====================" << std::endl;

	std::cout << "=== Unify ===" << std::endl;
	std::cout <<  t1 << std::endl;
	std::cout <<  t2 << std::endl;

	std::cout << "=== Result ===" << std::endl;
	prolog::result_bindings<> bindings;
	bool unifiable = prolog::unify(t1, t2, bindings);
	std::cout << "can unify: " << (unifiable ? "true" : "false") << std::endl;
	if (unifiable) {
		std::cout << "=== Bindings ===" << std::endl;
		std::cout << bindings;
	}

	std::cout << std::endl;
}

void test_bind_result(const prolog::term<>& t1, const prolog::term<>& t2) {
	std::cout << "====================" << std::endl;
	std::cout << "= Test bind_result =" << std::endl;
	std::cout << "====================" << std::endl;
	std::cout << "==== Unify ====" << std::endl;
	std::cout << t1 << std::endl;
	std::cout << t2 << std::endl;
	prolog::term<> working = t1;
	prolog::result_bindings<> bindings;

	bool unifiable = prolog::unify(working, t2, bindings);

	if(unifiable) {
		std::cout << "=== Bindings ===" << std::endl;
		std::cout << bindings;
		std::cout << "=== Bind to 1. ===" << std::endl;
		bindings.bind_results(working);
		bindings.bind_results(const_cast<prolog::term<>&>(t1)); // Shouldn't bind anything
		std::cout << "Original: " << t1 << std::endl; // Should be unchanged
		std::cout << "Binded  : " << working << std::endl;
	}
	else {
		std::cout << "!!! cannot even unify !!!" << std::endl;
	}
	std::cout << std::endl;
}

void test(const prolog::term<>& t1, const prolog::term<>& t2) {
	std::cout << "        *****" << std::endl;
	std::cout << "       *  " << test_id << "  *" << std::endl;
	std::cout << "        *****" << std::endl;
	std::cout << std::endl;
	test_unify(t1, t2);
	test_bind_result(t1, t2);
	test_id++;
}

int main2()
{
	prolog::variable_term<int> t1(56);
	prolog::composed_term<>	   t2("ahoj");

	prolog::variable_term<int> t3(59);
	prolog::variable_term<>	   t4("Variable");

	prolog::composed_term<> c1("father");
	c1.add_arg(t2);
	c1.add_arg(t4);

	prolog::composed_term<> c2("father");
	c2.add_arg(t2);
	c2.add_arg(t2);

	prolog::variable_term<> var("Result");

	test(t2, c1);

	test(var, t2);

	test(c1, c2);

	prolog::composed_term<> ahoj("ahoj");
	prolog::variable_term<> X("X");
	prolog::variable_term<> Y("Y");
	prolog::composed_term<> foo("foo");
	prolog::composed_term<> bar("bar");

	prolog::composed_term<> ahoj2("ahoj");
	prolog::composed_term<> ahoj3("ahoj");
	prolog::composed_term<> ahoj4("ahoj");

	ahoj.add_arg(X);
	ahoj.add_arg(foo);
	ahoj.add_arg(X);

	ahoj2.add_arg(Y);
	ahoj2.add_arg(Y);
	ahoj2.add_arg(foo);

	ahoj3.add_arg(Y);
	ahoj3.add_arg(foo);
	ahoj3.add_arg(Y);

	test(ahoj, ahoj2);

	ahoj4.add_arg(Y);
	ahoj4.add_arg(Y);
	ahoj4.add_arg(bar);

	test(ahoj, ahoj4);

	prolog::result_bindings<> bindings, bindings2;
	bool b = prolog::unify(ahoj, ahoj2, bindings);
	b = prolog::unify(ahoj, ahoj3, bindings2);

	std::cout << "bindings 1:" << std::endl;
	std::cout << bindings << std::endl;
	std::cout << "bindings 2:" << std::endl;
	std::cout << bindings2 << std::endl;
	std::cout << "merge (1->2):" << std::endl;
	bindings2.merge(bindings);
	std::cout << bindings2 << std::endl;
	std::cout << "value of X:" << std::endl;
	std::cout << bindings[X] << std::endl;
	std::cout << std::endl;

	for (const auto& bin : bindings)
		std::cout << bin.first << " -> " << bin.second.second<< std::endl;

	std::cout << "rule:" << std::endl;
	prolog::rule<> simple(std::move(ahoj));
	simple.add_term(std::move(foo));
	simple.add_term(std::move(bar));
	std::cout << simple << std::endl;

	std::cout << "it test:" << std::endl;
	prolog::rule<>::iterator it = simple.begin();
	prolog::composed_term<>& zk = *it;
	std::cout << zk << std::endl;
	auto it2 = it;
	it++;
	std::cout << zk << std::endl;
	std::cout << *it2 << std::endl;
	std::cout << *it << std::endl;
	std::cout << zk << std::endl;

	std::cout << "database:" << std::endl;
	prolog::database<> db;
	db.add_rule(std::move(simple));
	std::cout << db << std::endl;

	std::cout << "looking for:" << std::endl;
	std::cout << ahoj2 << std::endl;
	auto itlow = db.lower_bound(ahoj2);  // itlow points to b
	auto itup = db.upper_bound(ahoj2);   // itup points to e (not d)
				
	std::cout << "results:" << std::endl;
	// print range [itlow,itup):
	for (auto it = itlow; it != itup; ++it)
		std::cout << *it << std::endl;
	for (const auto& it3: db.begin()->head())
		std::cout << it3 << std::endl;

    return 0;
}

