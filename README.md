MiniPascalInterpreter
=====================

This is an interpreter of poor subset of Pascal programming language, named as MiniPascal by me.
I.e. procedures, classes, modules are not supported. Only simple arithmetical and logical instructions
in the single source file are allowed. You can check the sample program test_sources/source.pas.
It can be usable as educational parser :)
Also, program is fully crossplatform with nice UTF-8 support.


MiniPascal syntax (in the Backus–Naur Form)
=====================
P ::= program D1; Â.
D1 ::= var D2 {;D2}
D2 ::= I {,I} : [int, bool]
Â ::= begin S {;S} end
S ::= I := E | if E then S | if E then S else S | while E do S | B | read (I) | write (E)
E :: = E1 | E1 [=, <>, <, <=, >, >=] E1
El ::= T | T + E1 | T - E1 | T ^ E1
T ::= F | F * T | F / T | F ^ T
F ::= I | N | L | not F | un F |(E)
L ::= true | false
I  ::= Letter| ILetter | IDigit
N ::= Digit | NDigit


Build requirements
=====================
1) C++11-compatible compiler (Microsoft Visual Studio 2010 or higher, gcc 4.6 or higher)
2) PoCo library (pocoproject.org) - should be accessable by the compiler


Build under Windows
=====================
Just open the MiniPascalInterpreter.sln solution file in your Visual Studio.

Build under Unix-like
=====================
<TODO:>


Usage
=====================
MiniPascalInterpreter [-h] [-v] [-l] [-p] source1.pas source2.pas ... sourceN.pas
NOTE: Under Windows one can use /option syntax, i.e. /v instead of -v.

__Option name_____|______Option description________________________________________________________
-h, --help        | Display brief help information about command line arguments
-v, --verbose     | Allow detailed output for lexer, parser/semler and POLIR converter and interpreter
-l, --lexeme-file | Save extracted from the source lexeme table to the specified text file
-p, --polir-file  | Save converted from the lexemes POLIR data to the specified text file
___________________________________________________________________________________________________


Implementation details
=====================
Program is written on the C++ language, using Standard Template Library (http://www.cplusplus.com/reference/stl/)
and PoCo library (http://www.pocoproject.org)