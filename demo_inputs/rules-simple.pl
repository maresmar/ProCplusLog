% Prolog build-in
member(X,[X|_]).
member(X,[_|T]) :- member(X,T).

muz(jara).
muz(ondra).
muz(david).
muz(jindra).
muz(honza).
muz(pavel).

zena(pavla).
zena(eva).
zena(hana).

rodina(pavla, jara, [ondra, david, jindra, honza, eva, hana, pavel]).

zeny([S|E],[S|X]):-zena(S),zeny(E,X).
zeny([S|E],X):-muz(S),zeny(E,X).
zeny([],[]).


muzi([S|E],[S|X]):-muz(S),muzi(E,X).
muzi([S|E],X):-zena(S),muzi(E,X).
muzi([],[]).

deti(Kdo, X):- rodina(_,_,X),member(Kdo, X).

bratri(Kdo, X):- deti(Kdo, Deti), muzi(Deti, X).

zkouska :- !.