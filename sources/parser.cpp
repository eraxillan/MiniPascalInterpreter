/**
 * @file
 * @brief The implementation of syntax analyzer
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "lexer.h"
#include "parser.h"

using namespace MiniPascal;

void
MpParser::ERR (const std::string& _text)
{
	m_logstream.error () << "[" << m_iCurrLine << "] " << "Syntax error: " << _text << std::endl;

	UnicodeConsole::instance ().pause ();
	exit (1);
}

void
MpParser::ERR2 (const std::string& _text)
{
	m_logstream.error () << "[" << m_iCurrLine << "] " << "Semantic error: " << _text << std::endl;

	UnicodeConsole::instance ().pause ();
	exit (2);
}

void
MpParser::GC ()
{
	m_sCurrLexeme = m_lexer->getNextLexeme (&m_iCurrLine);
}

void
MpParser::INFO (const std::string& _name) const
{
	m_logstream.debug () << _name << " (" << m_sCurrLexeme << ")" << std::endl;
}

//
// P ::= program D1; В.
//
void
MpParser::P ()
{
	INFO ("P");

	//
	// "Program" keyword
	//
	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_PROGRAM)) != 0)
		ERR ("Keyword \"program\" expected");
	else
		GC ();

	//
	// Variable block (also check for ";")
	//
	D1 ();

	//
	// Code block
	//
	B (true);

	//
	// Check for "."
	//
	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_PROGRAM_END)) != 0)
	{
		ERR ("\".\" expected");
	}
}

//
// D1 ::= var D2 {;D2}
//
void
MpParser::D1 ()
{
	INFO ("D1");

	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_VAR)) != 0)
		ERR ("Keyword \"var\" expected.");
	else
		GC ();

	D2 ();
}

//
// D2 ::= I {,I} : [int, bool]
//
void
MpParser::D2 ()
{
	INFO ("D2");

	if (m_validVars.find (m_sCurrLexeme) != m_validVars.end ())
		ERR2 ("Duplicate identifier");

	//
	// Mark variable as already declared
	//
	// DEBUG:
	//cout << "DEBUG: "<< "Declare variable " << CH << endl;
	m_validVars [m_sCurrLexeme].clear ();
	GC ();

	//
	// One or more variables are present?
	//
	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_COMMA)))
	{
		GC ();
		D2 ();
		return;
	}

	//
	// Read the variable data type info
	//
	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_TYPE)) != 0)
		ERR ("\":\" expected");
	else
		GC ();

	//
	// Validate the variable data type
	//
	if ((Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_BOOL)) != 0)
		&& (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_INT)) != 0))
	{
		ERR ("Unknown variable data type");
	}

	//
	// Add type info
	//
	for (MpStringsDict::iterator i = m_validVars.begin (); i != m_validVars.end (); ++i)
	{
		if (i->second.empty ())
			i->second = m_sCurrLexeme;
	}
	GC ();

	//
	// Check for ";"
	//
	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_OPERATOR_END)) != 0)
	{
		ERR ("\";\" expected");
	}
	else
		GC ();

	//
	// Another "var" block is present?
	//
	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_BEGIN)) != 0)
		D2 ();
}

//
// В ::= begin S {;S} end
//
void
MpParser::B (bool main)
{
	INFO ("B");

	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_BEGIN)) != 0)
		ERR ("Keyword \"begin\" expected");
	else
		GC ();

	while ((Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_END)) != 0) && !m_sCurrLexeme.empty ())
	{
		// DEBUG:
		//cout << "-------------------" << endl;
		S ();

		if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_END)))
			break;

		if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_CLOSE_BRACKET)))
			ERR ("\"(\" expected.");

		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_OPERATOR_END)) != 0)
		{
			// DEBUG:
			//cout << "DEBUG: " << "In B (), CH = " << CH << endl;
			ERR ("\";\" expected");
		}
		else
			GC ();

		// DEBUG:
		//cout << "--------------------" << endl;
	}

	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_END)) != 0)
		ERR ("Keyword \"end\" expected");

	GC ();
}

//
// S ::= I := E | if E then S | if E then S else S | while E do S | B | read (I) | write (E)
//
void
MpParser::S ()
{
	INFO ("S");

	//
	// Syntax: if E then S else S
	//
	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_IF)))
	{
		GC ();
		E ();

		//
		// Check data type: only boolean expression can be used in "if"
		//
		std::string t = m_exprOpType.top ();
		m_exprOpType.pop ();
		if (Poco::UTF8::icompare (t, m_lexer->getKeyword (KEYWORD_BOOL)) != 0)
			ERR2 ("\"if\" statement require bool expression");

		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_THEN)) != 0)
			ERR ("Keyword \"then\" expected");
		else
			GC ();

		S ();

		if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_OPERATOR_END)))
			return;

		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_ELSE)) != 0)
			ERR ("Keyword \"else\" or \";\" expected");
		else
			GC ();

		S ();
		return;
	}

	//
	// do S while E
	//
	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_DO)))
	{
		GC ();
		S ();
		GC ();

		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_WHILE)) != 0)
			ERR ("Keyword \"while\" expected");
		else
			GC ();

		E ();

		//
		// Check data type: only bool variable can be used in "while"
		//
		std::string t = m_exprOpType.top ();
		m_exprOpType.pop ();
		if (Poco::UTF8::icompare (t, m_lexer->getKeyword (KEYWORD_BOOL)) != 0)
			ERR2 ("\"while\" statement require bool expression");

		return;
	}

	//
	// begin S; S; ... end
	//
	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_BEGIN)))
	{
		B (false);
		return;
	}

	//
	// read (I)
	//
	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_READ)))
	{
		GC ();
		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_OPEN_BRACKET)) != 0)
			ERR ("\"(\" expected");
		else
			GC ();

		I ();

		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_CLOSE_BRACKET)) != 0)
			ERR ("\")\" expected");
		else
			GC ();

		return;
	}

	//
	// write (E)
	//
	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_WRITE)))
	{
		GC ();

		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_OPEN_BRACKET)) != 0)
			ERR ("\"(\" expected.");
		else
			GC ();

		E ();

		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_CLOSE_BRACKET)) != 0)
			ERR ("\")\" expected.");
		else
			GC ();

		return;
	}

	//
	// I := E
	// Check identifier: it must be declared earlier in the "var" block
	//
	if (m_validVars.find (m_sCurrLexeme) == m_validVars.end ())
		ERR2 ("Unknown identifier");

	I ();

	//
	// Check for ":="
	//
	if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_ASSUME)) != 0)
		ERR ("\":=\" expected");
	else
		GC ();

	E ();

	//
	// Check for types mismatch
	//
	std::string t1, t2;
	t1 = m_exprOpType.top ();
	m_exprOpType.pop ();
	t2 = m_exprOpType.top ();
	m_exprOpType.pop ();
	if (Poco::UTF8::icompare (t1, t2) != 0)
		ERR2 ("Type mismatch in assign operator");
}

//
// E :: = E1 | E1 [=, <>, <, <=, >, >=] E1
//
void
MpParser::E ()
{
	INFO ("E");

	E1 ();

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_EQUAL))
		|| !Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_NOT_EQUAL))
		|| !Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_LESSER))
		|| !Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL))
		|| !Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_MORE))
		|| !Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		E1 ();

		//
		// Pop type1, type2, op and push typeResult
		//
		std::string t1, t2, op;
		t1 = m_exprOpType.top ();
		m_exprOpType.pop ();
		op = m_exprOpType.top ();
		m_exprOpType.pop ();
		t2 = m_exprOpType.top ();
		m_exprOpType.pop ();

		//
		// Compare operators need only t1 = t2
		//
		if (!Poco::UTF8::icompare (t1, t2))
			m_exprOpType.push (m_opTypes [op].typeResult);
		else
			ERR2 ("Type mismatch: operation " + op + " need equal types.");

		return;
	}
}

//
// El ::= T | T + E1 | T - E1 | T ^ E1
//
void
MpParser::E1 ()
{
	INFO ("E1");
	T ();

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_PLUS))
		|| !Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_MINUS)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		E1 ();
		checkTypes ();

		// DEBUG:
		//cout << "ADD EXP END." << endl;

		return;
	}

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_OR)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		E1 ();

		checkTypes ();

		// DEBUG:
		//cout << "OR EXP END." << endl;

		return;
	}
}

//
// T ::= F | F * T | F / T | F ^ T
//
void
MpParser::T ()
{
	INFO ("T");
	F ();

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_MUL))
		|| !Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter(DELIM_DIV)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		T ();
		checkTypes ();
		return;
	}

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_AND)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		T ();
		checkTypes ();
		return;
	}
}

//
// F ::= I | N | L | not F | un F |(E)
//
void
MpParser::F ()
{
	INFO ("F");

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_NOT)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		F ();

		std::string t = m_exprOpType.top ();
		m_exprOpType.pop ();
		if (Poco::UTF8::icompare (t, m_lexer->getKeyword (KEYWORD_BOOL)) != 0)
			ERR2 ("\"not\" operator needs bool operand");
		//
		// Remove "not" from stack
		//
		m_exprOpType.pop ();
		//
		// Push result type to stack
		//
		m_exprOpType.push (m_lexer->getKeyword (KEYWORD_BOOL));

		return;
	}

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_UN)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		F ();

		//
		// Check types
		//
		std::string t = m_exprOpType.top ();
		m_exprOpType.pop ();
		if (Poco::UTF8::icompare (t, m_lexer->getKeyword (KEYWORD_INT)) != 0)
			ERR2 ("un operator needs int operand");
		//
		// Remove "un" from stack
		//
		m_exprOpType.pop ();
		//
		// Put result type to the stack
		//
		m_exprOpType.push (m_lexer->getKeyword (KEYWORD_INT));

		return;
	}

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_OPEN_BRACKET)))
	{
		GC ();
		E ();

		if (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getDelimiter (DELIM_CLOSE_BRACKET)) != 0)
			ERR ("\")\" expected");
		else
			GC ();

		return;
	}

	if (!Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_TRUE))
		|| !Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_FALSE)))
	{
		L ();
		return;
	}

	if (m_digits.find (m_sCurrLexeme[0] - 47) != m_digits.end ())
	{
		N ();
		return;
	}

	I ();
}

//
// L ::= true | false
//
void
MpParser::L ()
{
	INFO ("L");

	m_exprOpType.push (m_lexer->getKeyword (KEYWORD_BOOL));

	if ((Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_TRUE)) != 0)
		&& (Poco::UTF8::icompare (m_sCurrLexeme, m_lexer->getKeyword (KEYWORD_FALSE)) != 0))
		ERR ("Type mismatch");
	else
		GC ();
}

//
// I  ::= Letter| ILetter | IDigit
//
void
MpParser::I ()
{
	INFO ("I");

	// DEBUG:
	//cout << "DEBUG: " << "CH = " << CH << endl;

	if (m_letters.find (m_sCurrLexeme [0]) == m_letters.end ())
		ERR ("Invalid identifier");

	if (m_validVars.find (m_sCurrLexeme) == m_validVars.end ())
		ERR ("Unknown identifier");
	else
	{
		m_exprOpType.push (m_validVars [m_sCurrLexeme]);

		if (m_usedVars.find (m_sCurrLexeme) == m_usedVars.end ())
			m_usedVars.insert (m_sCurrLexeme);

		GC ();
	}
}

//
// N ::= C | NC
//
void
MpParser::N ()
{
	INFO ("N");

	m_exprOpType.push (m_lexer->getKeyword (KEYWORD_INT));

	int num;
	if (stringIsInt (m_sCurrLexeme, num))
		GC ();
	else
		ERR ("NaN");
}

void
MpParser::checkTypes ()
{
	//
	// Pop type1, type2, op and push typeResult
	//
	std::string t1, t2, op;
	t1 = m_exprOpType.top ();
	m_exprOpType.pop ();
	op = m_exprOpType.top ();
	m_exprOpType.pop ();
	t2 = m_exprOpType.top ();
	m_exprOpType.pop ();

	//
	// Use binary operators types table
	//
	if (!Poco::UTF8::icompare (t1, m_opTypes [op].type1)
		&& !Poco::UTF8::icompare (t2, m_opTypes [op].type2))
	{
		m_exprOpType.push (m_opTypes [op].typeResult);
	}
	else
		ERR2 ("Type mismatch: operation " + op
		+ " need types " + m_opTypes [op].type1
		+ " and " + m_opTypes [op].type2);
}

MpParser::MpParser (MpLexer* _lexer, Poco::LogStream& _ls) : m_logstream (_ls)
{
	m_lexer = _lexer;
	m_lexer->setToBegin ();
	m_iCurrLine = 0;

	unsigned char i;
	for (i = 0; i <= 9; i ++)
		m_digits.insert (i);

	std::string ls = "abcdefghijklmnopqrstuvwxyz";
	for (i = 0; i < ls.length (); i ++)
		m_letters.insert (ls [i]);

	//
	// Filling operation types array (for binary ones)
	//
	MpOpTypes opt;

	//
	// :=
	//
	opt.type1.clear ();
	opt.type2.clear ();
	opt.equal = true;
	opt.typeResult.clear ();
	m_opTypes [m_lexer->getDelimiter (DELIM_ASSUME)] = opt;

	//
	// +, -, *, /
	//
	opt.type1 = m_lexer->getKeyword (KEYWORD_INT);
	opt.type2 = m_lexer->getKeyword (KEYWORD_INT);
	opt.equal = false;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_INT);
	m_opTypes [m_lexer->getDelimiter (DELIM_PLUS)] = opt;

	opt.type1 = m_lexer->getKeyword (KEYWORD_INT);
	opt.type2 = m_lexer->getKeyword (KEYWORD_INT);
	opt.equal = false;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_INT);
	m_opTypes [m_lexer->getDelimiter (DELIM_MINUS)] = opt;

	opt.type1 = m_lexer->getKeyword (KEYWORD_INT);
	opt.type2 = m_lexer->getKeyword (KEYWORD_INT);
	opt.equal = false;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_INT);
	m_opTypes [m_lexer->getDelimiter (DELIM_MUL)] = opt;

	opt.type1 = m_lexer->getKeyword (KEYWORD_INT);
	opt.type2 = m_lexer->getKeyword (KEYWORD_INT);
	opt.equal = false;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_INT);
	m_opTypes [m_lexer->getDelimiter (DELIM_DIV)] = opt;

	//
	// AND, OR
	//
	opt.type1 = m_lexer->getKeyword (KEYWORD_BOOL);
	opt.type2 = m_lexer->getKeyword (KEYWORD_BOOL);
	opt.equal = false;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getKeyword (KEYWORD_AND)] = opt;

	opt.type1 = m_lexer->getKeyword (KEYWORD_BOOL);
	opt.type2 = m_lexer->getKeyword (KEYWORD_BOOL);
	opt.equal = false;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getKeyword (KEYWORD_OR)] = opt;

	//
	// Compare operators
	//
	opt.type1.clear ();
	opt.type2.clear ();
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_EQUAL)] = opt;

	opt.type1.clear ();
	opt.type2.clear ();
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_NOT_EQUAL)] = opt;

	opt.type1.clear ();
	opt.type2.clear ();
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_MORE)] = opt;

	opt.type1.clear ();
	opt.type2.clear ();
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL)] = opt;

	opt.type1.clear ();
	opt.type2.clear ();
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_LESSER)] = opt;

	opt.type1.clear ();
	opt.type2.clear ();
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL)] = opt;
}

bool
MpParser::loadLexemeTable (MpLexer* l)
{
	m_lexer = l;
	m_lexer->setToBegin ();
	return true;
}

//
// Check language syntax and semantic corectness
//
void
MpParser::parse ()
{
	m_sCurrLexeme = m_lexer->getNextLexeme (&m_iCurrLine);

	//
	// Call procedure which check source for "begin" keyword and start parsing
	//
	P ();

	//
	// No syntax errors! (In other case program exit)
	//
	m_logstream.debug () << "PARSER INFO: No syntax error found!" << std::endl;

	//
	// Check for unused variables
	//
	int warningCount = 0;
	for (MpStringsDict::iterator i = m_validVars.begin (); i != m_validVars.end (); ++i)
	{
		if (m_usedVars.find (i->first) == m_usedVars.end ())
		{
			m_logstream.warning () << "SEMLER WARNING: " << i->first << " unreferenced local variable" << std::endl;
			warningCount ++;
		}
	}

	if (warningCount)
		m_logstream.debug () << "SEMLER INFO: No errors, " << warningCount << " warnings" << std::endl;
	else
		m_logstream.debug () << "SEMLER INFO: No errors, no warnings." << std::endl;
}
