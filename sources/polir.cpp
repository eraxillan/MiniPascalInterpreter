/**
 * @file
 * @brief The implementation of POLIR converter and interpreter
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "polir.h"
using namespace std;
using namespace MiniPascal;

//
// Convert expression(s) to POLIR (Edsger Wybe Dijkstra algorithm) and execute it
//
// NOTE: does not check syntax and semantics, this should be done by the MpParser object
//

//
// Class constructor: fills priorities table and connect to MpLexer and MpParser objects
//
MpPolir::MpPolir (MpLexer* _lexer, MpParser* _parser, Poco::LogStream& _logstream) : m_logstream (_logstream)
{
	connectLexer (_lexer);
	connectParser (_parser);

	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_OPEN_BRACKET),    0));
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_CLOSE_BRACKET),   1));

	//
	// Fill operations priority table
	//
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_ASSUME),          2));
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (KEYWORD_OR),            3));
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (KEYWORD_AND),           4));
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (KEYWORD_NOT),           5));
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_LESSER),          6));  // <
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL), 6));  // <=
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_MORE),            6));  // >
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL),   6));  // >=
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_EQUAL),           6));  // =
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_NOT_EQUAL),       6));  // <>
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_PLUS),            7));  // +
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_MINUS),           7));  // -
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_MUL),             8));  // *
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (DELIM_DIV),             8));  // /
	m_opPriors.insert (std::make_pair (m_lexer->getDelimiter (KEYWORD_UN),            9));  // unary -
}

//
// Return operation priority
//
int MpPolir::priority (const std::string& op) const
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
MpPolir::convertExpression (const std::string& startL, bool* bIsConst)
{
	std::string lexeme = startL;

	//
	// What lexemes are the expression end markers?
	//
	set <std::string, less<std::string>> endDelims;
	endDelims.insert (m_lexer->getKeyword (KEYWORD_WHILE));
	endDelims.insert (m_lexer->getKeyword (KEYWORD_THEN));
	endDelims.insert (m_lexer->getKeyword (KEYWORD_ELSE));
	endDelims.insert (m_lexer->getKeyword (KEYWORD_END));
	endDelims.insert (m_lexer->getDelimiter (DELIM_OPERATOR_END));

	//
	// If at least one variable exists in expression, it will be set to "false"
	//
	if (bIsConst)
		(*bIsConst) = true;

	do
	{
		if (!Poco::UTF8::icompare (lexeme, m_lexer->getDelimiter (DELIM_CLOSE_BRACKET)))
		{
			while (Poco::UTF8::icompare (m_opStack.top (), m_lexer->getDelimiter (DELIM_OPEN_BRACKET)) != 0)
			{
				m_polirExpr.push_back (m_opStack.top ());
				m_opStack.pop ();
			}

			//
			// Delete the open bracket
			//
			m_opStack.pop ();
		}

		//
		// If ID or number (bool are internally stored as 0 or 1, but lexeme should be "true" or "false")
		//
		if ((Poco::Unicode::isAlpha (lexeme [0]) || Poco::Unicode::isDigit (lexeme [0]) || (lexeme [0] == '-'))
			&& (m_opPriors.find (lexeme) == m_opPriors.end ()))
		{
			m_polirExpr.push_back (lexeme);

			//
			// Is it const or variable?
			//
			if (bIsConst)
			{
				if (Poco::Unicode::isAlpha (lexeme [0])
					&& (Poco::UTF8::icompare (lexeme, m_lexer->getKeyword (KEYWORD_TRUE)) != 0)
					&& (Poco::UTF8::icompare (lexeme, m_lexer->getKeyword (KEYWORD_FALSE)) != 0))
				{
					(*bIsConst) = false;
				}
			}
		}

		if (!Poco::UTF8::icompare (lexeme, m_lexer->getDelimiter (DELIM_OPEN_BRACKET)))
			m_opStack.push (m_lexer->getDelimiter (DELIM_OPEN_BRACKET));

		//
		// l is operation
		//
		if ((m_opPriors.find (lexeme) != m_opPriors.end ())
			&& (Poco::UTF8::icompare (lexeme, m_lexer->getDelimiter (DELIM_OPEN_BRACKET)) != 0)
			&& (Poco::UTF8::icompare (lexeme, m_lexer->getDelimiter (DELIM_CLOSE_BRACKET)) != 0))
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
	} while (endDelims.find (lexeme) == endDelims.end ());	// while lexeme is not an terminator delimeter

	while (!m_opStack.empty ())
	{
		m_polirExpr.push_back (m_opStack.top ());
		m_opStack.pop ();
	}

	// DEBUG:
	//cout << "DEBUG: " << "convertExpression, end: L = " << l << endl;
}

void
MpPolir::convert (const std::string& lexeme)
{
	//
	// Check for type - arithmetic expression or complex such as "if" statement
	//
	char type = 0;

	std::string l = lexeme;
	if (!Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_IF)))
		type = 1;
	if (!Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_DO)))
		type = 2;
	if (!Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_READ)))
		type = 3;
	if (!Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_WRITE)))
		type = 4;
	if (!Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_BEGIN)))
		type = 5;
	if (!Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_END)))
		type = 6;
	if (!Poco::UTF8::icompare (l, m_lexer->getDelimiter (DELIM_PROGRAM_END)))
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
			if (b)
				m_logstream.warning () << "POLIR WARNING: In \"if\" operator, line " << lineIndex << ", constant condition was found." << std::endl;

			m_polirExpr.push_back ("_");             // p1 !F - p1 will be defined later
			p1Index = m_polirExpr.size () - 1;
			m_polirExpr.push_back ("!F");

			l = m_lexer->getNextLexeme (NULL);
			convert (l);                                  // S1 until else OR ;.

			l = m_lexer->getLexeme (m_lexer->getCurrentLexemeIndex ());

			if (!Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_ELSE)))
			{
				m_polirExpr.push_back ("_");         // p2 is now unknown.
				size_t p2Index = m_polirExpr.size () - 1;
				m_polirExpr.push_back ("!");

				l = m_lexer->getNextLexeme (NULL);
				p1 = m_polirExpr.size ();

				convert (l);                             // S2 until end or ;.
				size_t p2 = m_polirExpr.size ();

				//
				// Update indexes
				//
				std::stringstream ss;
				std::string s1, s2;
				ss << p1 << " " << p2;
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
				std::stringstream ss;
				std::string s1;
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
			if (b)
				m_logstream.warning () << "POLIR WARNING: In \"do\" operator, line " << lineIndex << ", constant condition was found." << std::endl;

			m_polirExpr.push_back (m_lexer->getKeyword (KEYWORD_NOT));

			std::stringstream ss;
			std::string s1;
			ss << p1 << " ";
			ss >> s1;
			m_polirExpr.push_back (s1);
			m_polirExpr.push_back ("!F");
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
				l = m_lexer->getNextLexeme (NULL);
				// DEBUG:
				//cout << " l = " << l << endl;
			} while ((Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_END)) != 0) && !l.empty ());

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
	// Search for the code block
	//
	std::string l;
	do
	{
		l = m_lexer->getNextLexeme (NULL);
	} while (Poco::UTF8::icompare (l, m_lexer->getKeyword (KEYWORD_BEGIN)) != 0);
	//
	// Now l points to code block begin
	//

	//
	// Convert operators to POLIR sequentially
	//
	do
	{
		convert (l);
	} while (!(l = m_lexer->getNextLexeme (0)).empty ());

	m_logstream.debug () << "POLIR: Convertion done, no errors" << std::endl;
	for (std::size_t i = 0; i < m_polirExpr.size (); i ++)
		m_logstream.debug () << m_polirExpr [i] << " ";
	m_logstream.debug () << endl;
}

void
MpPolir::executeProgram ()
{
	//
	// Operands stack: contain integer/boolean constants and integer/boolean variables
	//
	stack <std::string> varStack;

	unsigned int i = 0;
	while (i < m_polirExpr.size ())
	{
		if (m_polirExpr [i] == "!F")
		{
			//
			// bool_const index !F
			//
			std::string sIndex = varStack.top ();         // index
			varStack.pop ();
			std::string condition = varStack.top ();      // condition
			varStack.pop ();

			if ((Poco::UTF8::icompare (condition, m_lexer->getKeyword (KEYWORD_FALSE)) != 0)
				&& (Poco::UTF8::icompare (condition, "0") != 0))
			{
				i ++;
				continue;
			}

			i = stringToInt (m_logstream, sIndex);
			continue;
		}

		if (m_polirExpr [i] == "!")
		{
			//
			// index !
			//
			std::string sIndex = varStack.top ();
			varStack.pop ();

			i = stringToInt (m_logstream, sIndex);
			continue;
		}

		//
		// +, -, *, /, and, or, not, un, <, <=, =, >=, >, :=, read, write
		// Check for binary operation, type int
		//
		if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_PLUS))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_MINUS))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_MUL))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_DIV)))
		{
			int x, y;
			std::string sY = varStack.top ();
			varStack.pop ();
			std::string sX = varStack.top ();
			varStack.pop ();

			if (Poco::Unicode::isDigit (sX [0]) || (sX [0] == '-'))
				x = stringToInt (m_logstream, sX);
			else
				x = m_vars [sX].value;

			if (Poco::Unicode::isDigit (sY [0]) || (sY [0] == '-'))
				y = stringToInt (m_logstream, sY);
			else
				y = m_vars [sY].value;

			int result = 0;
			std::string sResult;

			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_PLUS)))
			{
				//
				// x y +
				//
				result = x + y;
			}

			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_MINUS)))
			{
				//
				// x y -
				//
				result = x - y;
			}

			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_MUL)))
			{
				//
				// x y *
				//
				result = x * y;
			}

			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_DIV)))
			{
				//
				// x y /
				//
				if (!y)
				{
					m_logstream << "RUNTIME ERROR: divide by zero" << std::endl;
					UnicodeConsole::instance ().pause ();
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
		if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getKeyword (KEYWORD_AND))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getKeyword (KEYWORD_OR)))
		{
			int x, y;
			std::string sY = varStack.top ();
			varStack.pop ();
			std::string sX = varStack.top ();
			varStack.pop ();

			if ((sX == "0") || !Poco::UTF8::icompare (sX, m_lexer->getKeyword (KEYWORD_FALSE)))
				x = 0;
			else
			{
				if ((sX == "1") || !Poco::UTF8::icompare (sX, m_lexer->getKeyword (KEYWORD_TRUE)))
					x = 1;
				else
					x = m_vars [sX].value;
			}

			if ((sY == "0") || !Poco::UTF8::icompare (sY, m_lexer->getKeyword (KEYWORD_FALSE)))
				y = 0;
			else
			{
				if ((sY == "1") || !Poco::UTF8::icompare (sY, m_lexer->getKeyword (KEYWORD_TRUE)))
					y = 1;
				else
					y = m_vars [sY].value;
			}

			int result = 0;
			std::string sResult;

			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getKeyword (KEYWORD_AND)))
			{
				//
				// x y and
				//
				result = x && y;
			}

			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getKeyword (KEYWORD_OR)))
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
		if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getKeyword (KEYWORD_UN)))
		{
			// x un
			int x;
			std::string sX = varStack.top ();
			varStack.pop ();

			if (Poco::Unicode::isDigit (sX [0]) || sX [0] == '-')
				x = stringToInt (m_logstream, sX);
			else
				x = m_vars [sX].value;

			int result = x * (-1);
			std::string sResult = intToString (result);
			varStack.push (sResult);

			i ++;
			continue;
		}

		if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getKeyword (KEYWORD_NOT)))
		{
			//
			// x not
			//
			int x;
			std::string sX = varStack.top ();
			varStack.pop ();

			if ((sX == "0") || !Poco::UTF8::icompare (sX, m_lexer->getKeyword (KEYWORD_FALSE)))
				x = 0;
			else
			{
				if ((sX == "1") || !Poco::UTF8::icompare (sX, m_lexer->getKeyword (KEYWORD_TRUE)))
					x = 1;
				else
					x = m_vars [sX].value;
			}

			int result = !x;
			std::string sResult = intToString (result);
			varStack.push (sResult);

			i ++;
			continue;
		}

		if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_LESSER))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_EQUAL))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_NOT_EQUAL))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_MORE))
			|| !Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL)))
		{
			int x = 0, y = 0, result = 0;

			std::string sY = varStack.top ();
			varStack.pop ();
			std::string sX = varStack.top ();
			varStack.pop ();

			//
			// x, y can be any - const or var, bool or int
			//
			if ((sX == "0") || !Poco::UTF8::icompare (sX, m_lexer->getKeyword (KEYWORD_FALSE)))
				x = 0;
			else if ((sX == "1") || !Poco::UTF8::icompare (sX, m_lexer->getKeyword (KEYWORD_TRUE)))
				x = 1;
			else
			{
				if (Poco::Unicode::isDigit (sX [0]) || (sX [0] == '-'))
					x = stringToInt (m_logstream, sX);
				else
					x = m_vars [sX].value;
			}

			if ((sY == "0") || !Poco::UTF8::icompare (sY, m_lexer->getKeyword (KEYWORD_FALSE)))
				y = 0;
			else if ((sY == "1") || !Poco::UTF8::icompare (sY, m_lexer->getKeyword (KEYWORD_TRUE)))
				y = 1;
			else
			{
				if (Poco::Unicode::isDigit (sY [0]) || (sY [0] == '-'))
					y = stringToInt (m_logstream, sY);
				else
					y = m_vars [sY].value;
			}

			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_LESSER)))
			{
				//
				// x y <
				//
				result = (x < y);
			}
			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_LESSER_OR_EQUAL)))
			{
				//
				// x y <=
				//
				result = (x <= y);
			}
			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_EQUAL)))
			{
				//
				// x y =
				//
				result = (x == y);
			}
			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_NOT_EQUAL)))
			{
				//
				// x y <>
				//
				result = (x != y);
			}
			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_MORE)))
			{
				//
				// x y >
				//
				result = (x > y);
			}
			if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_MORE_OR_EQUAL)))
			{
				//
				// x y >=
				//
				result = (x >= y);
			}

			std::string sResult = intToString (result);
			varStack.push (sResult);
			i ++;
			continue;
		}

		if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getKeyword (KEYWORD_READ)))
		{
			//
			// id read
			//
			std::string name = varStack.top ();
			varStack.pop ();

			std::string out = "\"read\" function was called: please enter ";
			out += m_vars [name].type;
			out += " variable \"";
			out += name;
			out += "\" : ";
			UnicodeConsole::instance ().writeLine (out);

			//
			// Read the value as UTF-8, and convert it to the lower case
			//
			std::string value = UnicodeConsole::instance ().readLine ();
			Poco::UTF8::toLowerInPlace (value);

			//
			// Update variable value with user entered one
			//
			if (!Poco::UTF8::icompare (m_vars [name].type, m_lexer->getKeyword (KEYWORD_BOOL)))
			{
				if ((value == "0") || !Poco::UTF8::icompare (value, m_lexer->getKeyword (KEYWORD_FALSE)))
					m_vars [name].value = 0;
				else
					m_vars [name].value = 1;
			}

			if (!Poco::UTF8::icompare (m_vars [name].type, m_lexer->getKeyword (KEYWORD_INT)))
				m_vars [name].value = stringToInt (m_logstream, value);

			i ++;
			continue;
		}

		if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getKeyword (KEYWORD_WRITE)))
		{
			//
			// id || const write
			//
			std::string sX = varStack.top ();
			varStack.pop ();

			std::string out = "\"write\" function was called: the result is ";
			if (m_vars.find (sX) != m_vars.end ())
			{
				if (!Poco::UTF8::icompare (m_vars [sX].type, m_lexer->getKeyword (KEYWORD_BOOL)))
					out += (m_vars [sX].value ? "true" : "false");
				else
					out += intToString (m_vars [sX].value);
			}
			else
			{
				out += "\"";
				out += sX;
				out += "\"";
			}

			UnicodeConsole::instance ().writeLine (out);
			i ++;
			continue;
		}

		if (!Poco::UTF8::icompare (m_polirExpr [i], m_lexer->getDelimiter (DELIM_ASSUME)))
		{
			//
			// We haven't any result here, we just will update the variable value
			// id const :=
			//
			std::string s1 = varStack.top (); // Value.
			varStack.pop ();
			std::string s2 = varStack.top (); // Variable.
			varStack.pop ();

			//
			// Update value
			//
			if (!Poco::UTF8::icompare (s1, m_lexer->getKeyword (KEYWORD_FALSE)))
				m_vars [s2].value = 0;
			if (!Poco::UTF8::icompare (s1, m_lexer->getKeyword (KEYWORD_TRUE)))
			if (s1 == m_lexer->getKeyword (KEYWORD_TRUE))
				m_vars [s2].value = 1;
			if (Poco::Unicode::isDigit (s1 [0]) || (s1 [0] == '-'))
			{
				m_vars [s2].value = stringToInt (m_logstream, s1);

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

	m_logstream.debug () << "POLIR: Executing done! No errors found" << std::endl;
}

bool
MpPolir::saveToFile (const std::string& _file_name)
{
	try
	{
		Poco::FileOutputStream f (_file_name);

		for (unsigned int i = 0; i < m_polirExpr.size (); i++)
			m_logstream.debug () << m_polirExpr [i] << " ";
		m_logstream.debug () << std::endl;
	}
	catch (...)
	{
		m_logstream.error () << "POLIR: I/O error, could not open file " << _file_name << " in the write mode" << std::endl;
		return false;
	}

	return true;
}
