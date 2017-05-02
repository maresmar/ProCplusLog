Specifikace
===========
Program bude rozdělen na dvě části - na knihovnu pro práci s Prologem a na hlavní program, který bude sloužit pro pracování se vstupem a jeho parsování.

Knihovna
--------------
Knihovna pracuje s termy pomocí objektů. Předkem všech objektů je abstraktní typ `term`. Jeho potomky jsou `složený term` a `proměnná`. Složený term je označení pro libovolný složený term klasického Prologu např. `dummy(foo, bar(X))` nebo ale i pro atom např. `foo`. Při návrhu jsem se snažil zjednodušit návrh SWI-Prologu <http://stackoverflow.com/a/27649100/1392034>

Samotná knihovna umožňuje pomocí templatových parametrů zvolit typ pro atomy a funktory - umožňuje vytvořit `foo(bar)` ale i `52(12)`. Umožňuje to do budoucna rozšířit knihovnu o speciální typ, který má rychlejší porovnávání než `std::string`, ale který vypisuje něco hezčího než `int` při volání `operator<<`.

Tyto objekty knihovna umožňuje vytvářet, procházet a skládat. Zároveň, ale uživateli knihovny neumožňuje pracovat přímo s vnitřní reprezentací dat a chová se v tomto případě jako proxy třída nad modelem (pro zjednodušení práce s knihovnou). Zároveň, ale bohužel musí vnitřně ukládat data pomocí shared_ptrů, aby umožnila skládání a "línou unifikaci" (viz dále).

Jednotlivé termy knihovna umožňuje mezi sebou unifikovat. Jedná se ale o "línou unifikaci", kde nedochází ke změně termů, ale vytváří se pouze seznam párů "co se má unifikovat na co". Díky tomu není nutné při nemožnosti unifikace trávit hodně času kopírováním složených termů a následným mazáním termů z paměti. Výsledky této "líné unifikace" se uloží do vlastního objektu. Tyto výsledky je možné procházet a také mezi sebou slučovat (což odpovídá v pravidlu `rule(X,Y,Z):- fnc1(X), fnc2(X, Y), fnc3(X, Y, Z).` situaci po návratu z pravidla `fnc2` před voláním `fnc3`).

Dále knihovna umožňuje z výsledků "líné unifikace" vytvářet nové termy - například z `foo(X, bar(Y))` pro `X = bar(Y)` a `Y = bla` -> `foo(bar(bla), bar(bla))`. Zároveň, ale případné proměnné vytváří nově - např. `bar(foo(X), X)` pro `X = foo(X)` -> `foo(foo(X_2), foo(X_2))`, aby při unifikaci vzniklého termu s původním nedošlo k chybnému ovlivňování výsledku.

Dále knihovna umožňuje spojovat termy do pravidel - např. `rule(X):- foo(X), bar(X).`. Tyto pravidla umožňuje ukládat do databáze pravidel. V těchto pravidlech poté umožňuje procházet a efektivně vyhledávat (za použití signatury funkce - např. `(rule 1)`.

Dále knihovna umožňuje hledat v databázi všechna řešení daného termu (jako v SWI-Prologu). Knihovna k tomu používá vlastní objekt call stacku, kde si ukládá průchod databází, pozici v seznamu možných použitelných pravidel (=pravidla se stejnou signaturou) a předchozí výsledky unifikace (aby se v případě selhání unifikace mohla vrátit zpět a pokračovat dalším použitelným pravidlem). Knihovna umožňuje zaříznutí větve (prologové `!`), pomocí atomu se specifickým názvem (default constructor typu templaty).

Hlavní program
----------------------
Hlavní program přijme jako parameter soubor s databází pravidel, tyto pravidla parsuje a vloží do databáze. Poté se uživateli zobrazí vstupní řádek, kde uživatel zadá vlastní termy, na jejichž řešení se chce zeptat. Po zobrazení jednoho řešení může buď zadat nový vstup / zobrazit další řešení / skončit program.

Dále program bude zvládat přepersovat klasický format SWI-Prologového listu `[Head|Tail]` do prologového `list(Head, Tail)` a formát `[first, second, third]`. Dále zvládá anonymní proměnné - ve SWI-Prologu `_` a komentáře - řádky začínající na `%`. 

Formát pravidel a termů odpovídá SWI-Prologu (atomy a funktory začínají malým písmenem, proměnné velkým, v názvu může být libovolný alfanumerický znak, argumenty složeného termu jsou uvnitř závorek a jsou odděleny čárkou, na konci pravidla je tečka, proměnné mají platnost pouze v rámci jednoho pravidla).

    % Komentar
    term(atom,Promnena).
    pravidlo(X,Y,Z) :- term(atom, _), term2(X, Y, Z).

Podporované vlastnosti Prologu
----------------------------------------------
* Plná práce se složenými termy a pravidly
* Možná práce s listy (místo vestavěného "trikového" zpracovávání, podpora na úrovni prologu - místo  `[Head| Tail]` prostě `list(Head, Tail)`)
* Možno zavést prologové počítání a porovnávaní pomocí pravidel (při zavedení správných pravidel pro `+(X,Y)` - pak "3" = `+(1,+(1,1))` - ale bez podpory "trikového" počítání (vnitřního počítání v int)
* Práce anonymními proměnnými (`_`)

Nepodporované funkce
---------------------------------
* klíčová slova `fail`
* definice vlastních operátorů
* vestavěné počítání a porovnání mezi čísly - možné ale zavést pomocí pravidel přímo v Prologu
* termy pro práci se soubory
* termy pro práci s databází (vkládání a odebírání pravidel z databáze)
* termy pro práci se vstupem
* termy pro práci s výsledky unifikace (`bagof` apod...)