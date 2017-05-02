% rozdel(+Seznam, -S1, -S2, -S3, -S4, -S5):- rozdeli seznam na skoro
% stejne casti (+/-1), |Seznam| >= 5
rozdel(Seznam,A1,A2,A3,A4,A5):-podret(Seznam,S1,S2,S3,S4,_),
prefix(Seznam,S1,A1,Z1),prefix(Z1,S2,A2,Z2),prefix(Z2,S3,A3,Z3),
prefix(Z3,S4,A4,A5).

% podret(+Seznam, -S1, -S2, -S3, -S4, -S5):- vytori 5 podretezcu se skoro
% stejne dlouhymi castmi (+/-1), |Seznam| >= 5
podret([A1,A2,A3,A4,A5|T], [A1|T1], [A2|T2], [A3|T3], [A4|T4], [A5|T5]):- podret(T,T1,T2,T3,T4,T5).
podret([A1,A2,A3,A4,A5], [A1], [A2], [A3], [A4], [A5]).
podret([A1,A2,A3,A4], [A1], [A2], [A3], [A4], []).
podret([A1,A2,A3], [A1], [A2], [A3], [], []).
podret([A1,A2], [A1], [A2], [], [], []).
podret([A1], [A1], [], [], [], []).
podret([],[],[],[],[],[]).

%prefix(+Seznam, +Delka, -Prefix, -Zbytek):- Vrátí prvních Delka prvků seznamu
prefix([P|Zbytek],[ _|Len],[P|Prefix],Konec):-prefix(Zbytek,Len,Prefix,Konec).
prefix(Zbytek,[],[],Zbytek).

%trans(+Mat, -Trans):- Transponuje matici
trans(Mat,[Prvni|Zbytek]):-prvni(Mat,X,Prvni),trans(X,Zbytek).
trans([[]|_],[]).

%prvni(+Matice, -BezPrvnihoSloupce, -PrvniSloupec):- odstrani prvni prvek ze vsech seznamu a ulozi ho
prvni([],[],[]).
prvni([[Z|Tento]|Zbytek],[Tento|Dalsi],[Z|K]):-prvni(Zbytek, Dalsi,K).