% Prolog build-in
member(X,[X| _ ]).
member(X,[ _ |T]) :- member(X,T).

% This buffer is for notes you don't want to save.
% If you want to create a file, visit that file with C-x C-f,
% then enter the text in that file's own buffer.

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

select_all(Co, [Co|Konec], X):-select_all(Co,Konec,X).
select_all(Co, [S|E], [S|X]):- select_all(Co,E,X).
select_all(_, [], []).

zeny([S|E],[S|X]):-zena(S),zeny(E,X).
zeny([S|E],X):-muz(S),zeny(E,X).
zeny([],[]).


muzi([S|E],[S|X]):-muz(S),muzi(E,X).
muzi([S|E],X):-zena(S),muzi(E,X).
muzi([],[]).

deti(Kdo, X):- rodina(_,_,X),member(Kdo, X).

bratri(Kdo, X):- deti(Kdo, Deti), muzi(Deti, X).

nejstarsi_bratr(Kdo, Koho):-deti(Koho, Deti), nbz(Koho, Kdo, Deti).

nbz(Koho, Kdo, [Koho|E]):-nbz(Koho, Kdo, E).
nbz(Koho, Kdo, [S|E]):- zena(S),nbz(Koho, Kdo, E).


byti_starsim_bratrem(Kdo, Koho):- deti(Kdo, Deti), muz(Kdo), bsbz(Kdo, Koho,Deti).

byti_mladsim_bratrem(Kdo, Koho):- deti(Kdo, Deti), muz(Kdo), bmbz(Kdo, Koho, Deti).
bmbz(Kdo, Koho, [Koho|E]):- member(Kdo, E).

zkouska :- !.