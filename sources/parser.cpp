/**
 * @file
 * @brief The implementation of syntax analyzer
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "lexer.h"
#include "parser.h"

using namespace std;
using namespace MiniPascal;

void
MpParser::ERR (const MpString& text)
{
	setTextColor (MP_COLOR_ERROR);
	MpCout << _TEXT ("[") << m_iCurrLine << _TEXT ("] ") << _TEXT ("Syntax error: ") << text << endl;
	setTextColor (MP_COLOR_NORMAL);

	systemPause ();
	exit (1);
}

void
MpParser::ERR2 (const MpString& text)
{
	setTextColor(MP_COLOR_ERROR);
	MpCout << _TEXT ("[") << m_iCurrLine << _TEXT ("] ") << _TEXT ("Semantic error: ") << text << endl;
	setTextColor(MP_COLOR_NORMAL);

	systemPause ();
	exit (2);
}

void
MpParser::GC ()
{
	m_sCurrLexeme = m_lexer->getNextLexeme (&m_iCurrLine);
}

void
MpParser::INFO (const MpString& name) const
{
	// DEBUG:
	//cout << name << " (" << CH << ")" << endl;
}

//
// P ::= program D1; Â.
//
void
MpParser::P ()
{
	INFO (_TEXT ("P"));

	//
	// "Program" keyword
	//
	if (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_PROGRAM))
		ERR (_TEXT ("Keyword \"program\" expected."));
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
	if ( m_sCurrLexeme != m_lexer->getDelimiter (DELIM_PROGRAM_END) )
	{
		ERR (_TEXT ("\".\" expected."));
	}
}

//
// D1 ::= var D2 {;D2}
//
void
MpParser::D1 ()
{
	INFO (_TEXT ("D1"));

	if (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_VAR))
		ERR (_TEXT ("Keyword \"var\" expected."));
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
	INFO (_TEXT ("D2"));

	if (m_validVars.find (m_sCurrLexeme) != m_validVars.end ())
		ERR2 (_TEXT ("Duplicate identifier."));

	//
	// Mark variable as already declared
	//
	// DEBUG:
	//cout << "DEBUG: "<< "Declare variable " << CH << endl;
	m_validVars [m_sCurrLexeme] = _TEXT ("");
	GC ();

	//
	// One more variable is present?
	//
	if (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_COMMA))
	{
		GC ();
		D2 ();
		return;
	}

	//
	// Read the variable data type info
	//
	if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_TYPE))
		ERR (_TEXT ("\":\" expected."));
	else
		GC ();

	//
	// Validate the variable data type
	//
	if ( (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_BOOL)) && (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_INT)) )
	{
		ERR (_TEXT ("Unknown type."));
	}

	//
	// Add type info
	//
	for (MpStringsDict::iterator i = m_validVars.begin (); i != m_validVars.end (); ++i)
	{
		if (i->second == _TEXT (""))
			i->second = m_sCurrLexeme;
	}
	GC ();

	//
	// Check for ";"
	//
	if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_OPERATOR_END))
	{
		ERR (_TEXT ("\";\" expected."));
	}
	else GC ();

	//
	// Another "var" block is present?
	//
	if (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_BEGIN))
		D2 ();
}

//
// Â ::= begin S {;S} end
//
void
MpParser::B (bool main)
{
	INFO (_TEXT ("B"));

	if (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_BEGIN))
		ERR (_TEXT ("Keyword \"begin\" expected."));
	else
		GC ();

	while ((m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_END)) && (!m_sCurrLexeme.empty ()))
	{
		// DEBUG:
		//cout << "-------------------" << endl;
		S ();

		if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_END))
			break;

		if (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_CLOSE_BRACKET))
			ERR (_TEXT ("\"(\" expected."));

		if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_OPERATOR_END))
		{
			// DEBUG:
			//cout << "DEBUG: " << "In B (), CH = " << CH << endl;
			ERR (_TEXT ("\";\" expected."));
		}
		else
			GC ();

		// DEBUG:
		//cout << "--------------------" << endl;
	}

	if (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_END))
		ERR (_TEXT ("Keyword \"end\" expected."));

	GC ();
}

//
// S ::= I := E | if E then S | if E then S else S | while E do S | B | read (I) | write (E)
//
void
MpParser::S ()
{
	INFO (_TEXT ("S"));

	//
	// Syntax: if E then S else S
	//
	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_IF))
	{
		GC ();
		E ();

		//
		// Check data type: only bool variable can be used in "if"
		//
		MpString t = m_exprOpType.top ();
		m_exprOpType.pop ();
		if (t != m_lexer->getKeyword (KEYWORD_BOOL))
			ERR2 (_TEXT ("if statement require bool expression."));

		if (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_THEN))
			ERR (_TEXT ("Keyword \"then\" expected."));
		else
			GC ();

		S ();

		if (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_OPERATOR_END))
			return;

		if (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_ELSE))
			ERR (_TEXT ("Keyword \"else\" or \";\" expected."));
		else
			GC ();

		S ();
		return;
	}

	//
	// do S while E
	//
	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_DO))
	{
		GC ();
		S ();
		GC ();

		if (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_WHILE))
			ERR (_TEXT ("Keyword \"while\" expected."));
		else
			GC ();

		E ();

		//
		// Check data type: only bool variable can be used in "while"
		//
		MpString t = m_exprOpType.top ();
		m_exprOpType.pop ();
		if (t != m_lexer->getKeyword (KEYWORD_BOOL))
			ERR2 (_TEXT ("while statement require bool expression."));

		return;
	}

	//
	// begin S; S; ... end
	//
	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_BEGIN))
	{
		B (false);
		return;
	}

	//
	// read (I)
	//
	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_READ))
	{
		GC ();
		if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_OPEN_BRACKET) )
			ERR (_TEXT ("\"(\" expected."));
		else
			GC ();

		I ();

		if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_CLOSE_BRACKET))
			ERR (_TEXT ("\")\" expected."));
		else
			GC ();

		return;
	}

	//
	// write (E)
	//
	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_WRITE))
	{
		GC ();

		if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_OPEN_BRACKET))
			ERR (_TEXT ("\"(\" expected."));
		else
			GC ();

		E ();

		if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_CLOSE_BRACKET))
			ERR (_TEXT ("\")\" expected."));
		else
			GC ();

		return;
	}

	//
	// I := E
	// Check identifier: it must be declared earlier in "var" block
	//
	if (m_validVars.find (m_sCurrLexeme) == m_validVars.end ())
		ERR2 (_TEXT ("Unknown identifier."));

	I ();

	//
	// Check for ":="
	//
	if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_ASSUME) )
		ERR (_TEXT ("\":=\" expected."));
	else
		GC ();

	E ();

	//
	// Check for types mismatch
	//
	MpString t1, t2;
	t1 = m_exprOpType.top ();
	m_exprOpType.pop ();
	t2 = m_exprOpType.top ();
	m_exprOpType.pop ();
	if (t1 != t2)
		ERR2 (_TEXT ("Type mismatch in assign operator."));
}

//
// E :: = E1 | E1 [=, <>, <, <=, >, >=] E1
//
void
MpParser::E ()
{
	INFO (_TEXT ("E"));

	E1 ();

	if ((m_sCurrLexeme == m_lexer->getDelimiter (DELIM_EQUAL)) || (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_NOT_EQUAL) ) ||
		(m_sCurrLexeme == m_lexer->getDelimiter (DELIM_LESSER) ) || (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL) ) ||
		(m_sCurrLexeme == m_lexer->getDelimiter (DELIM_MORE) ) || (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		E1 ();

		//
		// Pop type1, type2, op and push typeResult
		//
		MpString t1, t2, op;
		t1 = m_exprOpType.top ();
		m_exprOpType.pop ();
		op = m_exprOpType.top ();
		m_exprOpType.pop ();
		t2 = m_exprOpType.top ();
		m_exprOpType.pop ();

		//
		// Compare operators need only t1 = t2
		//
		if (t1 == t2)
			m_exprOpType.push (m_opTypes [op].typeResult);
		else
			ERR2 (_TEXT ("Type mismatch: operation ") + op + _TEXT (" need equal types."));

		return;
	}
}

//
// El ::= T | T + E1 | T - E1 | T ^ E1
//
void
MpParser::E1 ()
{
	INFO (_TEXT ("E1"));
	T ();

	if ((m_sCurrLexeme == m_lexer->getDelimiter (DELIM_PLUS) ) || (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_MINUS)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		E1 ();
		checkTypes ();

		// DEBUG:
		//cout << "ADD EXP END." << endl;

		return;
	}

	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_OR))
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
	INFO (_TEXT ("T"));
	F ();

	if ( (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_MUL)) || (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_DIV)))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		T ();
		checkTypes ();
		return;
	}

	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_AND))
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
	INFO (_TEXT ("F"));

	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_NOT))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		F ();

		MpString t = m_exprOpType.top ();
		m_exprOpType.pop ();
		if (t != m_lexer->getKeyword (KEYWORD_BOOL))
			ERR2 (_TEXT ("not operator needs bool operand."));
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

	if (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_UN))
	{
		m_exprOpType.push (m_sCurrLexeme);

		GC ();
		F ();

		//
		// Check types
		//
		MpString t = m_exprOpType.top ();
		m_exprOpType.pop ();
		if (t != m_lexer->getKeyword (KEYWORD_INT))
			ERR2 (_TEXT ("un operator needs int operand."));
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

	if (m_sCurrLexeme == m_lexer->getDelimiter (DELIM_OPEN_BRACKET))
	{
		GC ();
		E ();

		if (m_sCurrLexeme != m_lexer->getDelimiter (DELIM_CLOSE_BRACKET))
			ERR (_TEXT ("\")\" expected."));
		else
			GC ();

		return;
	}

	if ((m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_TRUE)) || (m_sCurrLexeme == m_lexer->getKeyword (KEYWORD_FALSE)))
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
	INFO (_TEXT ("L"));

	m_exprOpType.push (m_lexer->getKeyword (KEYWORD_BOOL));

	if ((m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_TRUE)) && (m_sCurrLexeme != m_lexer->getKeyword (KEYWORD_FALSE)))
		ERR (_TEXT ("Type mismatch."));
	else
		GC ();
}

//
// I  ::= Letter| ILetter | IDigit
//
void
MpParser::I ()
{
	INFO (_TEXT ("I"));

	// DEBUG:
	//cout << "DEBUG: " << "CH = " << CH << endl;

	if (m_letters.find (m_sCurrLexeme [0]) == m_letters.end ())
		ERR (_TEXT ("Invalid identifier."));

	if (m_validVars.find (m_sCurrLexeme) == m_validVars.end ())
		ERR (_TEXT ("Unknown identifier."));
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
	INFO (_TEXT ("N"));

	m_exprOpType.push (m_lexer->getKeyword (KEYWORD_INT));

	int num;
	MpStringStream ss;
	ss << m_sCurrLexeme;
	ss >> num;
	if (ss.fail () || !ss.eof ())
		ERR (_TEXT ("Invalid number"));
	else
		GC ();
}

void
MpParser::checkTypes ()
{
	//
	// Pop type1, type2, op and push typeResult
	//
	MpString t1, t2, op;
	t1 = m_exprOpType.top ();
	m_exprOpType.pop ();
	op = m_exprOpType.top ();
	m_exprOpType.pop ();
	t2 = m_exprOpType.top ();
	m_exprOpType.pop ();

	//
	// Use binary operators types table
	//
	if ( (t1 == m_opTypes [op].type1) && (t2 ==  m_opTypes [op].type2) )
		m_exprOpType.push (m_opTypes [op].typeResult);
	else
		ERR2 (_TEXT ("Type mismatch: operation ") + op
		+ _TEXT (" need types ") + m_opTypes [op].type1
		+ _TEXT (" and ") + m_opTypes [op].type2);
}

MpParser::MpParser (MpLexer* pLex)
{
	m_lexer = pLex;
	m_lexer->setToBegin ();
	m_iCurrLine = 0;

	unsigned char i;
	for (i = 0; i <= 9; i ++)
		m_digits.insert (i);

	MpString ls = _TEXT ("abcdefghijklmnopqrstuvwxyz");
	for (i = 0; i < ls.length (); i ++)
		m_letters.insert (ls [i]);

	//
	// Filling operation types array (for binary ones)
	//
	MpOpTypes opt;

	//
	// :=
	//
	opt.type1 = _TEXT ("");
	opt.type2 = _TEXT ("");
	opt.equal = true;
	opt.typeResult = _TEXT ("");
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
	opt.type1 = _TEXT ("");
	opt.type2 = _TEXT ("");
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_EQUAL)] = opt;

	opt.type1 = _TEXT ("");
	opt.type2 = _TEXT ("");
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_NOT_EQUAL)] = opt;

	opt.type1 = _TEXT ("");
	opt.type2 = _TEXT ("");
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_MORE)] = opt;

	opt.type1 = _TEXT ("");
	opt.type2 = _TEXT ("");
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL)] = opt;

	opt.type1 = _TEXT ("");
	opt.type2 = _TEXT ("");
	opt.equal = true;
	opt.typeResult = m_lexer->getKeyword (KEYWORD_BOOL);
	m_opTypes [m_lexer->getDelimiter (DELIM_LESSER)] = opt;

	opt.type1 = _TEXT ("");
	opt.type2 = _TEXT ("");
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
	MpCout << _TEXT ("PARSER INFO: No syntax error found!") << endl;

	//
	// Check for unused variables
	//
	int warningCount = 0;
	for (MpStringsDict::iterator i = m_validVars.begin (); i != m_validVars.end (); ++i)
	{
		if ( m_usedVars.find (i->first) == m_usedVars.end () )
		{
			setTextColor(MP_COLOR_WARNING);
			MpCout << _TEXT ("SEMLER WARNING: ") << i->first << _TEXT (" unreferenced local variable.") << endl;
			setTextColor(MP_COLOR_NORMAL);
			warningCount ++;
		}
	}

	if (warningCount)
		MpCout << _TEXT ("SEMLER INFO: No errors, ") << warningCount << _TEXT (" warnings.") << endl;
	else
		MpCout << _TEXT ("SEMLER INFO: No errors, no warnings.") << endl;
}
