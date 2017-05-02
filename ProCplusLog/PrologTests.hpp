#ifndef PROLOG_TESTS_H
#define PROLOG_TESTS_H

#include "ProCplusLog.hpp"

void test_unify(const prolog::term<>& t1, const prolog::term<>& t2);

void test_bind_result(const prolog::term<>& t1, const prolog::term<>& t2);

void test(const prolog::term<>& t1, const prolog::term<>& t2);

int main();

#endif
