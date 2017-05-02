% Prolog build-in
member(X,[X|_]).
member(X,[_|T]) :- member(X,T).

man(adam).
man(peter).
man(paul).
 
woman(marry).
woman(eve).

parent(adam,peter):-!. % means adam is parent of peter
parent(eve,peter).
parent(adam,paul).
parent(marry,paul).

father(F,C):-man(F),parent(F,C).
mother(M,C):-woman(M),parent(M,C).


is_father(F):-father(F,_).
is_mother(M):-mother(M,_).

son(S,P):-man(S),parent(P,S).
daughter(D,P):-woman(D),parent(P,D).

grand_parent(G,N):-parent(G,X),parent(X,N).

human(H):-man(H).
human(H):-woman(H).

descendent(D,A):-parent(A,D).
descendent(D,A):-parent(P,D),descendent(P,A).

ancestor(A,D):-descendent(D,A).

% multiunificagtion test
foo(A, A, B, B, C, C).
% foo(M, N, T, U, N, T).