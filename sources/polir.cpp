/**
 * @file
 * @brief The implementation of POLIR converter and interpreter
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "polir.h"
using namespace std;
using namespace MiniPascal;

//
// Convert expression(s) to POLIR (Edsger Wybe Dijkstra algorithm) and execute it.
// DOES NOT CHECK SYNTAX AND SEMANTIC!
//

//
// Class constructor: fills priorities table and connect to MpLexer and MpParser objects
//
MpPolir::MpPolir (MpLexer * pLex, MpParser * pPsr)
{
	connectLexer (pLex);
	connectParser (pPsr);

	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_OPEN_BRACKET),    0));
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_CLOSE_BRACKET),   1));

	//
	// Fill operations priority table
	//
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_ASSUME),          2));
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (KEYWORD_OR),            3));
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (KEYWORD_AND),           4));
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (KEYWORD_NOT),           5));
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_LESSER),          6));  // <
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL), 6));  // <=
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_MORE),            6));  // >
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL),   6));  // >=
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_EQUAL),           6));  // =
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_NOT_EQUAL),       6));  // <>
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_PLUS),            7));    // +
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_MINUS),           7));    // -
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_MUL),             8));    // *
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (DELIM_DIV),             8));    // /
	m_opPriors.insert(std::make_pair(m_lexer->getDelimiter (KEYWORD_UN),            9));    // unary -
}

//
// Return operation priority
//
int MpPolir::priority (MpString op) const
{
	PriorityMap::const_iterator iOp = m_opPriors.find(op);
	return (iOp != m_opPriors.end()) ? iOp->second : -1;
}

//
// Connect and set up the MpLexer
//
void MpPolir::connectLexer (MpLexer * pLex)
{
	m_lexer = pLex;
	m_lexer->setToBegin ();
}

//
// Connect to MpParser and use it to fill MpVariable map
//
void
MpPolir::connectParser (MpParser * pPar)
{
	m_parser = pPar;

	//
	// Init variables
	//
	for (MpStringsDict::iterator i = m_parser->m_validVars.begin (); i != m_parser->m_validVars.end (); ++i)
	{
		MpVariable v;
		v.type = i->second;
		v.value = 0;

		m_vars [i->first] = v;
	}
}

void
MpPolir::convertExpression (const MpString& startL, bool* bIsConst)
{
	MpString lexeme = startL;

	//
	// What lexemes are the expression end markers?
	//
	set < MpString, less<MpString> > endDelims;
	endDelims.insert (m_lexer->getKeyword (KEYWORD_WHILE));
	endDelims.insert (m_lexer->getKeyword (KEYWORD_THEN));
	endDelims.insert (m_lexer->getKeyword (KEYWORD_ELSE));
	endDelims.insert (m_lexer->getKeyword (KEYWORD_END));
	endDelims.insert (m_lexer->getDelimiter (DELIM_OPERATOR_END));

	//
	// If at least one variable exists in expression, it will be set to "false"
	//
	if ( bIsConst )
		*bIsConst = true;

	do
	{
		if (lexeme == m_lexer->getDelimiter (DELIM_CLOSE_BRACKET))
		{
			while (m_opStack.top () != m_lexer->getDelimiter (DELIM_OPEN_BRACKET))
			{
				m_polirExpr.push_back (m_opStack.top ());
				m_opStack.pop ();
			}

			//
			// Delete (
			//
			m_opStack.pop ();
		}

		//
		// If ID or number (bool are stored as 0 or 1, but lexeme is 'true' or 'false')
		//
		if ((MpIsAlNum (lexeme [0]) || lexeme [0] == _TEXT('-')) && (m_opPriors.find (lexeme) == m_opPriors.end ()))
		{
			m_polirExpr.push_back (lexeme);

			//
			// Is it const or variable?
			//
			if (bIsConst)
			{
				if (MpIsAl (lexeme [0]) && (lexeme != _TEXT("true")) && (lexeme != _TEXT("false")))
					*bIsConst = false;
			}
		}

		if (lexeme == m_lexer->getDelimiter (DELIM_OPEN_BRACKET))
			m_opStack.push (m_lexer->getDelimiter (DELIM_OPEN_BRACKET));

		//
		// l is operation
		//
		if ((m_opPriors.find (lexeme) != m_opPriors.end ()) &&
			(lexeme != m_lexer->getDelimiter (DELIM_OPEN_BRACKET)) && (lexeme != m_lexer->getDelimiter (DELIM_CLOSE_BRACKET)))
		{
			if (m_opStack.empty ())
				m_opStack.push (lexeme);
			else
			{
				if (priority (m_opStack.top ()) < priority (lexeme))
					m_opStack.push (lexeme);
				else
				{
					while (!m_opStack.empty () && (priority (m_opStack.top ()) >= priority (lexeme)))
					{
						m_polirExpr.push_back (m_opStack.top ());
						m_opStack.pop ();
					}

					//
					// Push operation to stack
					//
					m_opStack.push (lexeme);
				}
			}
		}
		lexeme = m_lexer->getNextLexeme (NULL);
	} while (endDelims.find (lexeme) == endDelims.end ());

	while (!m_opStack.empty ())
	{
		m_polirExpr.push_back (m_opStack.top ());
		m_opStack.pop ();
	}

	// DEBUG:
	//cout << "DEBUG: " << "convertExpression, end: L = " << l << endl;
}

void
MpPolir::convert (const MpString & lexeme)
{
	//
	// Check for type - arithmetic expression or complex such as if
	//
	char type = 0;

	MpString l = lexeme;
	if (l == m_lexer->getKeyword (KEYWORD_IF))
		type = 1;
	if (l == m_lexer->getKeyword (KEYWORD_DO))
		type = 2;
	if (l == m_lexer->getKeyword (KEYWORD_READ))
		type = 3;
	if (l == m_lexer->getKeyword (KEYWORD_WRITE))
		type = 4;
	if (l == m_lexer->getKeyword (KEYWORD_BEGIN))
		type = 5;
	if (l == m_lexer->getKeyword (KEYWORD_END))
		type = 6;
	if (l == m_lexer->getDelimiter (DELIM_PROGRAM_END))
		return;

	long lineIndex = 0L;
	if (type > 0)
		l = m_lexer->getNextLexeme (&lineIndex);

	switch (type)
	{
		case 1:                               // if E then S1 {else S2}
		{
			unsigned int p1 = 0, p1Index = 0;

			//convert (l);                      // E until then.
			bool b;
			convertExpression (l, &b);
			if ( b )
			{
				setTextColor(MP_COLOR_WARNING);
				MpCout << _TEXT("POLIR WARNING: In \"if\" operator, line ")
					<< lineIndex << _TEXT (", constant condition was found.") << endl;
				setTextColor(MP_COLOR_NORMAL);
			}

			m_polirExpr.push_back (_TEXT ("_"));             // p1 !F - p1 defines later.
			p1Index = m_polirExpr.size () - 1;
			m_polirExpr.push_back (_TEXT ("!F"));

			l = m_lexer->getNextLexeme (NULL);
			convert (l);                                  // S1 until else OR ;.

			l = m_lexer->getLexeme (m_lexer->getCurrentLexemeIndex ());

			if (l == m_lexer->getKeyword (KEYWORD_ELSE))
			{
				m_polirExpr.push_back (_TEXT ("_"));         // p2 is now unknown.
				size_t p2Index = m_polirExpr.size () - 1;
				m_polirExpr.push_back (_TEXT ("!"));

				l = m_lexer->getNextLexeme (NULL);
				p1 = m_polirExpr.size ();

				convert (l);                             // S2 until end or ;.
				size_t p2 = m_polirExpr.size ();

				//
				// Update indexes
				//
				MpStringStream ss;
				MpString s1, s2;
				ss << p1 << _TEXT (" ") << p2;
				ss >> s1 >> s2;
				m_polirExpr [p1Index] = s1;
				m_polirExpr [p2Index] = s2;
			}
			else                               // ;.
			{
				p1 = m_polirExpr.size ();

				//
				// Update index
				//
				MpStringStream ss;
				MpString s1;
				ss << p1;
				ss >> s1;
				m_polirExpr [p1Index] = s1;
			}
			break;
		}

		case 2: // do S while E;
		{
			unsigned int p1;
			p1 = m_polirExpr.size ();

			convert (l);                       // S until while or end.
			l = m_lexer->getNextLexeme (NULL); // Skip ;.
			l = m_lexer->getNextLexeme (NULL); // Skip while.

			//convert (l);                     // E until ;.
			bool b;
			convertExpression (l, &b);
			if ( b )
			{
				setTextColor(MP_COLOR_WARNING);
				MpCout << _TEXT ("POLIR WARNING: In \"do\" operator, line ")
					   << lineIndex << _TEXT (", constant condition was found.") << endl;
				setTextColor(MP_COLOR_NORMAL);
			}

			m_polirExpr.push_back (m_lexer->getKeyword (KEYWORD_NOT));

			MpStringStream ss;
			MpString s1;
			ss << p1 << _TEXT (" ");
			ss >> s1;
			m_polirExpr.push_back (s1);
			m_polirExpr.push_back (_TEXT ("!F"));
			break;
		}
		case 3: // read (I);
		{
			//
			// Now l is (. Add to POLIR variable after it
			//
			m_polirExpr.push_back (m_lexer->getNextLexeme (NULL));

			m_lexer->getNextLexeme (NULL); // Skip ")".
			m_lexer->getNextLexeme (NULL); // Skip ";".

			m_polirExpr.push_back (m_lexer->getKeyword (KEYWORD_READ));
			break;
		}
		case 4: // write (E);
		{
			//
			// Now l is '(' - convert expression including it
			//
			convertExpression (l, NULL);

			m_polirExpr.push_back (m_lexer->getKeyword (KEYWORD_WRITE));
			break;
		}
		case 5: // begin S end
		{
			do
			{
				convert (l);
				// DEBUG:
				//cout << " l = " << l << endl;
			} while (((l = m_lexer->getNextLexeme (NULL)) != m_lexer->getKeyword (KEYWORD_END)) && (l != _TEXT ("")));
			m_lexer->getNextLexeme (NULL);
			break;
		}
		case 6: // end
		{
			break;
		}
		default:
		{
			//
			// Convert arithmetic expression
			//
			convertExpression (l, 0);
			break;
		}
	}
}

void
MpPolir::convertProgram ()
{
	//
	// Search for code block
	//
	MpString l;
	while ((l = m_lexer->getNextLexeme (NULL) ) != m_lexer->getKeyword (KEYWORD_BEGIN));
	//
	// Now l points to code block begin
	//

	//
	// Convert operators to POLIR sequentially
	//
	do
	{
		convert (l);
	} while ((l = m_lexer->getNextLexeme (0)) != _TEXT (""));

	MpCout << _TEXT ("POLIR: Convertion done, no errors.") << endl;
	for (std::size_t i = 0; i < m_polirExpr.size (); i ++)
		MpCout << m_polirExpr [i] << _TEXT (" ");
	MpCout << endl;
}

namespace
{
	//
	// Converts MpString to integer - return 0 if s isn't number
	//
	static int
	stringToInt (const MpString& s)
	{
		MpStringStream ss;
		ss << s;
		int i = 0;
		ss >> i;
		return i;
	}

	//
	// Converts integer to MpString
	//
	static MpString
	intToString (const int i)
	{
		MpStringStream ss;
		MpString s;
		ss << i;
		ss >> s;
		return s;
	}
}

void
MpPolir::executeProgram ()
{
	//
	// Operands stack: contain integer/boolean constants and integer/boolean variables
	//
	stack <MpString> varStack;

	unsigned int i = 0;
	while (i < m_polirExpr.size ())
	{
		if (m_polirExpr [i] == _TEXT ("!F"))
		{
			//
			// bool_const index !F
			//
			MpString sIndex = varStack.top ();         // index
			varStack.pop ();
			MpString condition = varStack.top ();      // condition
			varStack.pop ();

			if ((condition != m_lexer->getKeyword (KEYWORD_FALSE)) && (condition != _TEXT ("0")))
			{
				i ++;
				continue;
			}

			i = stringToInt (sIndex);
			continue;
		}

		if (m_polirExpr [i] == _TEXT ("!"))
		{
			//
			// index !
			//
			MpString sIndex = varStack.top ();
			varStack.pop ();

			i = stringToInt (sIndex);
			continue;
		}

		//
		// +, -, *, /, and, or, not, un, <, <=, =, >=, >, :=, read, write
		// Check for binary operation, type int.
		//
		if ((m_polirExpr [i] == m_lexer->getDelimiter (DELIM_PLUS)) ||
			(m_polirExpr [i] == m_lexer->getDelimiter (DELIM_MINUS)) ||
			(m_polirExpr [i] == m_lexer->getDelimiter (DELIM_MUL)) ||
			(m_polirExpr [i] == m_lexer->getDelimiter (DELIM_DIV)))
		{
			int x, y;
			MpString sY = varStack.top ();
			varStack.pop ();
			MpString sX = varStack.top ();
			varStack.pop ();

			if (MpIsDigit (sX [0]) || sX [0] == _TEXT ('-'))
				x = stringToInt (sX);
			else
				x = m_vars [sX].value;

			if (MpIsDigit (sY [0]) || sY [0] == _TEXT ('-'))
				y = stringToInt (sY);
			else
				y = m_vars [sY].value;

			int result = 0;
			MpString sResult;

			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_PLUS))
			{
				//
				// x y +
				//
				result = x + y;
			}

			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_MINUS))
			{
				//
				// x y -
				//
				result = x - y;
			}

			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_MUL))
			{
				//
				// x y *
				//
				result = x * y;
			}

			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_DIV))
			{
				//
				// x y /
				//
				if (!y)
				{
					setTextColor(MP_COLOR_ERROR);
					MpCout << _TEXT ("RUNTIME ERROR: divide by null.") << endl;
					setTextColor(MP_COLOR_NORMAL);
					system ("pause");
					exit (3);
				}

				result = (int) (x / y);
			}

			sResult = intToString (result);
			varStack.push (sResult);
			i ++;
			continue;
		}

		//
		// Check for binary operation, type bool
		//
		if ((m_polirExpr [i] == m_lexer->getKeyword (KEYWORD_AND)) || (m_polirExpr [i] == m_lexer->getKeyword (KEYWORD_OR)))
		{
			int x, y;
			MpString sY = varStack.top ();
			varStack.pop ();
			MpString sX = varStack.top ();
			varStack.pop ();

			if ((sX == _TEXT ("0")) || (sX == m_lexer->getKeyword (KEYWORD_FALSE)))
				x = 0;
			else
				if ((sX == _TEXT ("1")) || (sX == m_lexer->getKeyword (KEYWORD_TRUE)))
					x = 1;
				else
					x = m_vars [sX].value;

			if ((sY == _TEXT ("0")) || (sY == m_lexer->getKeyword (KEYWORD_FALSE)))
				y = 0;
			else
			{
				if ((sY == _TEXT ("1")) || (sY == m_lexer->getKeyword (KEYWORD_TRUE)))
					y = 1;
				else
					y = m_vars [sY].value;
			}

			int result = 0;
			MpString sResult;

			if (m_polirExpr [i] == m_lexer->getKeyword (KEYWORD_AND))
			{
				//
				// x y and
				//
				result = x && y;
			}

			if (m_polirExpr [i] == m_lexer->getKeyword (KEYWORD_OR))
			{
				//
				// x y or
				//
				result = x || y;
			}

			sResult = intToString (result);
			varStack.push (sResult);
			i ++;
			continue;
		}

		//
		// Check for unary operation
		//
		if (m_polirExpr [i] == m_lexer->getKeyword (KEYWORD_UN))
		{
			// x un
			int x;
			MpString sX = varStack.top ();
			varStack.pop ();

			if (MpIsDigit (sX [0]) || sX [0] == _TEXT ('-'))
				x = stringToInt (sX);
			else
				x = m_vars [sX].value;

			int result = x * (-1);
			MpString sResult = intToString (result);
			varStack.push (sResult);

			i ++;
			continue;
		}

		if (m_polirExpr [i] == m_lexer->getKeyword (KEYWORD_NOT))
		{
			//
			// x not
			//
			int x;
			MpString sX = varStack.top ();
			varStack.pop ();

			if ((sX == _TEXT ("0")) || (sX == m_lexer->getKeyword (KEYWORD_FALSE)))
				x = 0;
			else
			{
				if ((sX == _TEXT ("1")) || (sX == m_lexer->getKeyword (KEYWORD_TRUE)))
					x = 1;
				else
					x = m_vars [sX].value;
			}

			int result = ! x;
			MpString sResult = intToString (result);
			varStack.push (sResult);

			i ++;
			continue;
		}

		if ((m_polirExpr [i] == m_lexer->getDelimiter (DELIM_LESSER)) ||
			(m_polirExpr [i] == m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL)) ||
			(m_polirExpr [i] == m_lexer->getDelimiter (DELIM_EQUAL)) ||
			(m_polirExpr [i] == m_lexer->getDelimiter (DELIM_NOT_EQUAL)) ||
			(m_polirExpr [i] == m_lexer->getDelimiter (DELIM_MORE)) ||
			(m_polirExpr [i] == m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL)))
		{
			int x = 0, y = 0, result = 0;

			MpString sY = varStack.top ();
			varStack.pop ();
			MpString sX = varStack.top ();
			varStack.pop ();

			//
			// x, y can be any - const or var, bool or int
			//
			if ((sX == _TEXT ("0")) || (sX == m_lexer->getKeyword (KEYWORD_FALSE)))
				x = 0;
			else if ((sX == _TEXT ("1")) || (sX == m_lexer->getKeyword (KEYWORD_TRUE)))
				x = 1;
			else
			{
				if (MpIsDigit (sX [0]) || (sX [0] == _TEXT ('-')))
					x = stringToInt (sX);
				else
					x = m_vars [sX].value;
			}

			if ((sY == _TEXT ("0")) || (sY == m_lexer->getKeyword (KEYWORD_FALSE)))
				y = 0;
			else if ((sY == _TEXT ("1")) || (sY == m_lexer->getKeyword (KEYWORD_TRUE)))
				y = 1;
			else
			{
				if (MpIsDigit (sY [0]) || (sY [0] == _TEXT ('-')))
					y = stringToInt (sY);
				else
					y = m_vars [sY].value;
			}

			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_LESSER))
			{
				//
				// x y <
				//
				result = (x < y);
			}
			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL))
			{
				//
				// x y <=
				//
				result = (x <= y);
			}
			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_EQUAL))
			{
				//
				// x y =
				//
				result = (x == y);
			}
			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_NOT_EQUAL))
			{
				//
				// x y <>
				//
				result = (x != y);
			}
			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_MORE))
			{
				//
				// x y >
				//
				result = (x > y);
			}
			if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL))
			{
				//
				// x y >=
				//
				result = (x >= y);
			}

			MpString sResult = intToString (result);
			varStack.push (sResult);
			i ++;
			continue;
		}

		if (m_polirExpr [i] == m_lexer->getKeyword (KEYWORD_READ))
		{
			//
			// id read
			//
			MpString sX = varStack.top ();
			varStack.pop ();

			MpString sIn;
			MpCout << _TEXT ("read: enter ") << sX << _TEXT (" : ");
			MpCin >> sIn;

			//
			// Convert to lower case one
			//
			toLower (sIn);

			//
			// Update value
			//
			if (m_vars [sX].type == m_lexer->getKeyword (KEYWORD_BOOL))
			{
				if ((sIn == m_lexer->getKeyword (KEYWORD_FALSE)) || (sIn == _TEXT ("0")))
					m_vars [sX].value = 0;
				else
					m_vars [sX].value = 1;
			}

			if (m_vars [sX].type == m_lexer->getKeyword (KEYWORD_INT))
				m_vars [sX].value = stringToInt (sIn);

			i ++;
			continue;
		}

		if (m_polirExpr [i] == m_lexer->getKeyword (KEYWORD_WRITE))
		{
			//
			// id || const write
			//
			MpString sX = varStack.top ();
			varStack.pop ();
			if (m_vars.find (sX) != m_vars.end ())
			{
				if ( m_vars [sX].type == m_lexer->getKeyword (KEYWORD_BOOL) )
				{
					if ( m_vars [sX].value )
						MpCout << _TEXT ("write: ") << _TEXT ("true") << endl;
					else
						MpCout << _TEXT ("write: ") << _TEXT ("false") << endl;
				}
				else
					MpCout << _TEXT ("write: ") << m_vars [sX].value << endl;
			}
			else
				MpCout << _TEXT ("write: ") << sX << endl;

			i ++;
			continue;
		}

		if (m_polirExpr [i] == m_lexer->getDelimiter (DELIM_ASSUME))
		{
			//
			// We haven't any result, just update variable value
			// id const :=
			//
			MpString s1 = varStack.top (); // Value.
			varStack.pop ();
			MpString s2 = varStack.top (); // Variable.
			varStack.pop ();

			//
			// Update value
			//
			if (s1 == m_lexer->getKeyword (KEYWORD_FALSE))
				m_vars [s2].value = 0;
			if (s1 == m_lexer->getKeyword (KEYWORD_TRUE))
				m_vars [s2].value = 1;
			if (MpIsDigit (s1 [0]) || s1 [0] == _TEXT ('-'))
			{
				m_vars [s2].value = stringToInt (s1);

				// DEBUG:
				//cout << s2 << ":=" << stringToInt (s1) << endl;
			}

			i ++;
			continue;
		}

		//
		// Add variables and constants to stack
		//
		if (m_parser->m_opTypes.find (m_polirExpr [i]) == m_parser->m_opTypes.end ())
		{
			varStack.push (m_polirExpr [i]);

			i ++;
			continue;
		}
	}

	MpCout << _TEXT ("POLIR: Executing done! No errors found.") << endl;
}

bool
MpPolir::saveToFile (const MpChar* fileName)
{
	MpOutputFileStream f (fileName);
	if (!f)
	{
		MpCout << _TEXT ("POLIR: IO error, could not open file for write.") << endl;
		return false;
	}

	try
	{
		for (unsigned int i = 0; i < m_polirExpr.size (); i ++)
			MpCout << m_polirExpr [i] << _TEXT (" ");
		MpCout << endl;
	}
	catch (...)
	{
		MpCout << _TEXT ("POLIR: IO error, could not write to file.") << endl;
		return false;
	}
	f.close ();

	return true;
}
