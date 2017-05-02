# ProCplusLog
ProCplusLog is Prolog interpret and library written in C++. Work could be divided into two parts. First part is standalone Prolog library in C++ and second part is the user interface that uses this library. 

## User interface
### Syntax
The program uses standard SWI-Prolog syntax. Composed terms start with a lower case letter and variable with an upper case letter. You can use the list and anonymous variables.

~~~Prolog
% commnet line
% composed term with three variables and one atom
fact(Variable, atom, AnotherVariable, _).
% rules
rule(X,Y):-foo(X),bar(Y).
rule(X,_):-bar(X),!. % Prolog cut
% Example of list (same list in three forms)
sth([first,second,third]):-same([first,second|third]),same([first|[second|[third|[]]]]).
term(inner(foo),foo(foo(X))).
~~~

The Prologue cut `!` is also supported but you cannot use functions working with the database, infix functions or unify/non-unify function eg. `\=`.

### How to use app
The program needs command line argument, which specifies an input file. The internal rule database is created from this file. When the file is loaded, the application shows input line where you can write Prolog question.

~~~Prolog
?-first_question(X),second_question(Y).
~~~

The program could be exited by writing `halt.`

# Standalone Prolog library
Library developer entry point is in `ProCplusLog.hpp`. Here you can work with `composed_terms` and `variable_terms`. Every object has a template parameter which specifies the type of term functors and names of variables. So you can have `foo(bar)` or `51(10)` or whatever you want and has `operator==` (for printing it should also support `std::ostream& operator<< (std::ostream& stream, const T& sth)`)

~~~C++
prolog::composed_term<> foo("foo");
prolog::variable_term<> X("X");
prolog::variable_term<> Y("Y");
foo.add_arg(X);
foo.add_arg(Y);
// Creates foo(X,Y)

prolog::composed_term<> another("fool");
another.add_arg(prolog::composed_term<>("sth");
another.add_arg(prolog::variable_term<>("Var");

prolog::result_bindings<> bindings;
bool b = prolog::unify(foo, another, bindings); // = true

for (const auto& bin : bindings)
    if(bin.second.second.is_default_construed(); // is binded to variable? (or term)
        std::cout << bin.first << " -> " << bin.second.second<< std::endl; // Prints binded variable
    else
        std::cout << bin.first << " -> " << bin.second.second<< std::endl; // Prints binded term
/*
X -> sth
Y -> M
M -> M
*/
~~~
For more examples and demos see [PrologTests.cpp file](https://github.com/mmrmartin/ProCplusLog/blob/master/ProCplusLog/PrologTests.cpp). You can also use `rules` and `database` to store terms and search terms. And `solver` for looking for solutions (gives you results to your question based on the database).
