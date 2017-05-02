arc(a,b).		arc(b,c).
arc(a,c).		arc(a,d).
arc(b,e).		arc(e,f).
arc(b,f).		arc(f,g).	

% is there any path from X to Y 
path(X,Y):- arc(X,Y).
path(X,Y):- arc(X,Z),path(Z,Y).

% ?- pathall(a,g,R).
% R = [a,b,b,e,e,f,f,g]
% R = [a,b,b,f,f,g]

pathall(X,X,[]).
pathall(X,Y,[X,Z|L]):- arc(X,Z),pathall(Z,Y,L).
% pathall(a, e, X).


% ?- add(a,[b,c,d],Sol).
% X = [a,b,c,d]
% X = [b,a,c,d]
% X = [b,c,a,d]
% X = [b,c,d,a]
% ?- permut([a,b,c],X).
% X = [a,b,c]
% X = [b,a,c]
% X = [b,c,a]
% X = [a,c,b]
% X = [c,a,b]
% X = [c,b,a]

add(El,Tail,[El|Tails]).
add(X,[L|H],[L|R]):- add(X,H,R).

permut([],[]).
permut([L|H],R):- permut(H,R1),add(L,R1,R).
