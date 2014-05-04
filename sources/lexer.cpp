/**
 * @file
 * @brief The implementation of lexical analyzer
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "lexer.h"

using namespace std;
using namespace MiniPascal;

namespace
{
	static void
	trim (string& line)
	{
		//
		// Skip spaces in begin
		//
		unsigned int i = 0;
		while ( (' ' == line [i]) && (i < line.length ())) i ++;
		line.erase (0, i);
		if (!line.length ()) return;

		//
		// Skip spaces in end
		//
		i = line.length () - 1;
		while ( ((' ' == line [i]) || ('\n' == line [i])) && (i != 0))
			i --;
		// cout << i << endl;
		line.erase (i + 1, line.length () - i);
	}

	static void
	trim (wstring& line)
	{
		//
		// Skip spaces in begin
		//
		unsigned int i = 0;
		while ((i < line.length ()) && (L' ' == line [i]))
			i ++;
		line.erase (0, i);
		if (!line.length ())
			return;

		//
		// Skip spaces in end
		//
		i = line.length () - 1;
		while ( ((L' ' == line [i]) || (L'\n' == line [i])) && (i != 0))
			i --;
		line.erase (i + 1, line.length () - i);
	}
}

bool
MpLexer::skipComments (MpInputFileStream& f, MpString& line, long& lineIndex)
{
	//
	// Skip multiline comments
	//
	int iL, iR;
	iL = line.find (m_mlComments [0], 0);
	iR = line.find (m_mlComments [1], 0);

	if (iL >= 0)
	{
		// DEBUG:
		//cout << "[" << lineIndex << "]" << " " << "MultiLine Comment begin found." << endl;
		if ( (iR >= (int) m_mlComments [1].length ()) && (iR != -1) )
		{
			//
			// Singleline valid comment
			//
			line.erase (iL, iR - iL + m_mlComments [1].length ());

			// DEBUG:
			//cout << "[" << lineIndex << "]" << " " << "MultiLine Comment end found." << endl;
		}
		else
		{
			//
			// Maybe multiline comment, skip lines
			//
			while (!f.eof ())
			{
				getline (f, line);
				lineIndex ++;

				iR = line.find (m_mlComments [1], 0);
				if (iR >= 0)
				{
					//
					// Valid comment end
					//
					// DEBUG:
					//cout << "[" << lineIndex << "]" << " " << "MultiLine Comment end found." << endl;

					line.erase (0, iR + m_mlComments [1].length ());
					break;
				}
			}

			if (f.eof ())
			{
				//
				// Invalid comment!
				//
				MpCout << _TEXT ("[") << lineIndex << _TEXT ("]")
					<< _TEXT (" LEXER ERROR: ") << _TEXT ("Invalid comment - close symbol not found!") << endl;
				f.close ();
				return false;
			}
		}
	}

	//
	// Is it line comment?
	//
	iL = line.find (m_slComments [0], 0);
	if ( iL >= 0 )
	{
		// DEBUG:
		//cout << "[" << lineIndex << "]" << " " << "SingleLine comment found." << endl;

		line.erase (iL, line.length () - 1);
	}

	//
	// Return success
	//
	return true;
}

bool
MpLexer::isNumber (const MpString& token, int& num)
{
	MpStringStream ss;
	ss << token;
	ss >> num;
	if (ss.fail () || !ss.eof ())
		return false;

	return true;
}

bool
MpLexer::isKeyword (const MpString& token, int& index) const
{
	int i = 0;
	for ( ; i < 100; i ++)
	{
		if (token == m_keywords [i])
		{
			index = i;
			return true;
		}
	}

	return false;
}

bool
MpLexer::isDelimiter (const MpString& token, int& index)
{
	int i = 0;
	for ( ; (i < MP_ARR_LEN) && (m_delimeters [i].length ()); i ++)
	{
		if (token == m_delimeters [i])
		{
			index = i;
			return true;
		}
	}

	return false;
}

bool
MpLexer::writeToTable (const MpString& token, const long& lineIndex)
{
	if (token.empty ())
		return false;

	//DEBUG:
	//cout << "WTT, Token = " << token << endl;

	//
	// Token is number (table 3)
	//
	int num = 0;
	if (isNumber (token, num))
	{
		//
		// Use the hash function
		//
		long i = (long)(MP_ARR_LEN * num * 1.618) % MP_ARR_LEN;

		if (!m_pArrNumber [i].number)
		{
			//
			// The table cell is empty
			//
			m_pArrNumber [i].number = num;
			m_pArrNumber [i].count ++;
		}
		else
		{
			if (m_pArrNumber [i].number == num)
				m_pArrNumber [i].count ++;
			else
			{
				//
				// Handle the collision
				//
				long l = 1;

				while ((m_pArrNumber [(i + l*l + l + 1) % MP_ARR_LEN].number != num) &&
					(m_pArrNumber [(i + l*l + l + 1) % MP_ARR_LEN].number != 0)) l ++;

				m_pArrNumber [(i + l*l + l + 1) % MP_ARR_LEN].number = num;
				m_pArrNumber [(i + l*l + l + 1) % MP_ARR_LEN].count ++;
				i += l;
			}
		}

		//
		// Add to index table
		//
		m_pArrIndex [m_nLexemCount].i = 3;         // Number of table.
		m_pArrIndex [m_nLexemCount].j = i;         // Table index.
		m_pArrIndex [m_nLexemCount].k = lineIndex; // Lexem's line index.
		m_nLexemCount ++;
		if (m_nLexemCount >= MP_ARR_LEN)
		{
			MpCout << _TEXT ("LEXER ERROR: Too many lexems!") << endl;
			return false;
		}
		return true;
	}

	//
	// Token is delimiter (table 2)
	//
	int index = 0;
	if (isDelimiter (token, index))
	{
		//
		// Add to index table
		//
		m_pArrIndex [m_nLexemCount].i = 2;
		m_pArrIndex [m_nLexemCount].j = index;
		m_pArrIndex [m_nLexemCount].k = lineIndex;
		m_nLexemCount ++;
		if (m_nLexemCount >= MP_ARR_LEN)
		{
			MpCout << _TEXT ("LEXER ERROR: Too many lexems!") << endl;
			return false;
		}
		return true;
	}

	//
	// Token is keyword (table 1)
	//
	index = 0;
	if (isKeyword (token, index))
	{
		//
		// Add to index table
		//
		m_pArrIndex [m_nLexemCount].i = 1;
		m_pArrIndex [m_nLexemCount].j = index;
		m_pArrIndex [m_nLexemCount].k = lineIndex;
		m_nLexemCount ++;
		if (m_nLexemCount >= MP_ARR_LEN)
		{
			MpCout << _TEXT ("LEXER ERROR: Too many lexems!") << endl;
			return false;
		}
		return true;
	}

	//
	// Else token is ID (table 4)
	//
	long n = token.length ();

	//
	// Check token for errors
	//
	if (MpIsDigit (token [0]))
	{
		MpCout << _TEXT ("[") << lineIndex << _TEXT ("]")
			<< _TEXT (" LEXER ERROR: ") << _TEXT ("invalid ID ") << token << _TEXT (".") << endl;
		return false;
	}
	for (long j = 0L; j < n; j ++)
	{
		if (!MpIsAlNum (token [j]))
		{
			cout << _TEXT ("[") << lineIndex << _TEXT ("]")
				<< _TEXT (" LEXER ERROR: ") << _TEXT ("invalid char ") << token [j] << _TEXT (".") << endl;
			return false;
		}
	}

	//
	// Evaluate the hash function value
	//
	long i = (long) ((token [0] + token [n / 2] + token [n - 1]) % MP_ARR_LEN);

	if (!m_pArrID [i].id.length ())
	{
		//
		// The table cell is empty
		//
		m_pArrID [i].id = token;
		m_pArrID [i].count ++;
	}
	else
	{
		if (m_pArrID [i].id == token)
			m_pArrID [i].count ++;
		else
		{
			//
			// Handle the collision
			//
			long l = 1;
			while ((m_pArrID [(i + l) % MP_ARR_LEN].id != token) &&
				(m_pArrID [(i + l) % MP_ARR_LEN].id != _TEXT (""))) l++;

			m_pArrID [(i + l) % MP_ARR_LEN].id = token;
			m_pArrID [(i + l) % MP_ARR_LEN].count ++;
			i += l;
		}
	}

	//
	// Add new lexeme to the index table
	//
	m_pArrIndex [m_nLexemCount].i = 4;
	m_pArrIndex [m_nLexemCount].j = i;
	m_pArrIndex [m_nLexemCount].k = lineIndex;
	m_nLexemCount ++;
	if (m_nLexemCount >= MP_ARR_LEN)
	{
		cout << _TEXT ("[") << lineIndex << _TEXT ("]")
			<< _TEXT (" LEXER ERROR: ") << _TEXT ("Too many lexems!") << endl;
		return false;
	}

	return true;
}

MpLexer::MpLexer ()
{
	m_iCurrLexeme = 0;

	//
	// Allocate the memory for numbers, identifiers, indeces
	//
	m_pArrNumber = new MpNumLexeme [MP_ARR_LEN];
	m_pArrID = new MpIdLexeme [MP_ARR_LEN];
	m_pArrIndex = new MpIndexLexeme [MP_ARR_LEN];

	for (long i = 0L; i < MP_ARR_LEN; i ++)
	{
		m_pArrNumber [i].count = 0;
		m_pArrNumber [i].number = 0;
		m_pArrID [i].count = 0;
		m_pArrID [i].id = _TEXT ("");
		m_pArrIndex [i].i = 0;
		m_pArrIndex [i].j = 0;
		m_pArrIndex [i].k = 0;
	}

	m_nLexemCount = 0;
	m_bZeroIndex = false;
}

MpLexer::~MpLexer ()
{
	//
	// Free the previously allocated memory
	//
	delete [] m_pArrNumber;
	delete [] m_pArrID;
	delete [] m_pArrIndex;
}

bool
MpLexer::loadConfig (const MpChar* name)
{
	MpConfigFile cf;

	if (!cf.openFile (name))
	{
		MpCout << _TEXT ("LEXER I/O ERROR: Could not find language specs file!") << endl;
		system ("PAUSE");
		return 1;
	}

	m_keywords.clear();
	if (!cf.readSection (_TEXT ("[Keywords]"), m_keywords))
	{
		MpCout << _TEXT ("LEXER IO ERROR: Keywords section not found!") << endl;
		return false;
	}

	if (!cf.readSection (_TEXT ("[Delimiters]"), m_delimeters))
	{
		MpCout << _TEXT ("LEXER IO ERROR: Delimiters section not found!") << endl;
		return false;
	}

	if (!cf.readSection (_TEXT ("[Singleline]"), m_slComments))
	{
		MpCout << _TEXT ("LEXER IO ERROR: Singleline section not found!") << endl;
		return false;
	}

	if (!cf.readSection (_TEXT ("[Multiline]"), m_mlComments))
	{
		MpCout << _TEXT ("LEXER IO ERROR: Multiline section not found!") << endl;
		return false;
	}

	cf.closeFile ();

	for (int i = 0; i < MP_ARR_LEN; i++)
	{
		if (m_keywords.size() < MP_ARR_LEN)
			m_keywords.push_back (_TEXT (""));
		if (m_delimeters.size() < MP_ARR_LEN)
			m_delimeters.push_back (_TEXT (""));
		if (m_slComments.size() < MP_ARR_LEN)
			m_slComments.push_back (_TEXT (""));
		if (m_mlComments.size() < MP_ARR_LEN)
			m_mlComments.push_back (_TEXT (""));
	}

	return true;
}

bool
MpLexer::loadFile (const MpChar* name)
{
	long lineCounter = 0L, i = 0L;

	MpInputFileStream file (name);
	if (!file)
	{
		MpCout << _TEXT ("LEXER IO ERROR: Couldn't open file ") << name << _TEXT (".") << endl;
		return false;
	}

	//
	// 0) Extract numbers and ID's from file
	//
	while (!file.eof ())
	{
		MpString line = _TEXT (""), token = _TEXT (""), sd = _TEXT ("");
		int index = 0;

		getline (file, line);
		for (i = 0; i < (int) line.length (); i ++)
			line [i] = tolower (line [i]);
		// DEBUG:
		//cout << "Line = " << line << " " << line.length () << endl;

		lineCounter ++;
		i = 0;

		//
		// Clear line from comments
		//
		if (!skipComments (file, line, lineCounter))
			return false;
		if (line.length () <= 1)
			continue;

		//
		// ...And from spaces
		//
		trim (line);
		if (line.empty ())
			continue;

		// DEBUG:
		//cout << "[" << lineCounter << "] " << line << " | " << line.length () << endl;

		// Read next token.
		i = 0;
		token = _TEXT ("");

		bool tokenFound = false, delimFound = false;

		while (i <= (int) line.length ())
		{
			if (tokenFound)
			{
				// DEBUG:
				//cout << "Token = " << token << endl;
				/*
				int num = 0, index = 0;
				if (isNumber (token, num))
				cout << "Number (3) = " << num << endl;
				else
				if (isKeyword (token, index))
				cout << "Keyword (1) = " << token << endl;
				else
				cout << "ID (4)= " << token << endl;
				*/

				if (!writeToTable (token, lineCounter))
					return false;

				token = _TEXT ("");
				tokenFound = false;
			}

			if (delimFound)
			{
				// DEBUG:
				//cout << "Delim (2)= " << sd << endl;
				writeToTable (sd, lineCounter);

				sd = _TEXT ("");
				delimFound = false;
			}
			if (i == line.length ())
				break;

			//
			// Skip spaces
			//
			if (line [i] == ' ')
			{
				//
				// Token is ready
				//
				if (!token.empty ())
				{
					tokenFound = true;
					continue;
				}

				while (line [i] == _TEXT (' '))
					i ++;
			}

			//
			// Multi-symbol delimiter
			// NOTE: currently only two-symbol delims are supported
			//
			if (i <= (int) line.length () - 2)
			{
				sd = _TEXT ("");
				sd += line [i];
				sd += line [i + 1];

				if (isDelimiter (sd, index))
				{
					//
					// Token is ready
					//
					if (!token.empty ())
						tokenFound = true;

					delimFound = true;
					i ++;
					i ++;

					continue;
				}
			}

			//
			// One-symbol delimiter
			//
			sd = _TEXT ("");
			sd = line [i];
			if (isDelimiter (sd, index))
			{
				//
				// Token is ready
				//
				if (!token.empty ())
					tokenFound = true;

				delimFound = true;
				i ++;

				continue;
			}

			if ((i + 1) == line.length ())
			{
				if (!token.empty ())
					tokenFound = true;
			}

			token += line [i];
			i ++;
		} //end line cycle
	}

	file.close ();

	MpCout << _TEXT ("LEXER INFO: No errors!") << endl;
	return true;
}

bool
MpLexer::saveLexemeFile (const MpChar* name) const
{
	MpOutputFileStream fLexems (name);
	MpOutputFileStream fNumbers (_TEXT ("numbers.txt"));
	MpOutputFileStream fIDs (_TEXT ("IDs.txt"));

	// DEBUG:
	//cout << "ID and Num Table:" << endl;

	long i = 0L;
	for (i = 0L; i < MP_ARR_LEN; i ++)
	{
		if (m_pArrNumber [i].count)
		{
			// DEBUG:
			//cout << "[" << i << "] " << "Number = " << arrNumber [i].number << endl;

			fNumbers << m_pArrNumber [i].number << " " << i << endl;
		}
		if (m_pArrID [i].count)
		{
			// DEBUG:
			//cout << "[" << i << "] " << "ID = " << arrID [i].id << endl;

			fIDs << m_pArrID [i].id << " " << i << endl;
		}
	}

	// DEBUG:
	//cout << "Index Table:" << endl;

	for (i = 0L; i < MP_ARR_LEN; i++)
	{
		if (m_pArrIndex [i].i)
		{
			// DEBUG:
			//cout << "[" << i << "] " << "(k1, k2, k3) = " << "(" <<
			//arrIndex [i].i << ", " << arrIndex [i].j + 1 << ", " << arrIndex [i].k << ")" << endl;

			//
			// Add (k1, k2, k3) to lexem's file
			//
			fLexems << m_pArrIndex [i].i << _TEXT (" ") << m_pArrIndex [i].j << _TEXT (" ") << m_pArrIndex [i].k << endl;
		}
	}
	fLexems.close ();
	fNumbers.close ();
	fIDs.close ();

	return true;
}

MpString
MpLexer::getNextLexeme (long* lineIndex)
{
	if (m_bZeroIndex)
	{
		m_iCurrLexeme = 0;
		m_bZeroIndex = false;
	}
	if (lineIndex)
		*lineIndex = m_pArrIndex [m_iCurrLexeme].k;

	switch (m_pArrIndex [m_iCurrLexeme].i)
	{
		//
		// Number
		//
		case 3:
		{
			MpStringStream ss;
			ss << m_pArrNumber [m_pArrIndex [m_iCurrLexeme].j].number;
			m_iCurrLexeme ++;
			return ss.str ();
		}
		//
		// Identifier
		//
		case 4:
		{
			m_iCurrLexeme ++;
			return m_pArrID [m_pArrIndex [m_iCurrLexeme - 1].j].id;
		}
		//
		// Delimiter
		//
		case 2:
		{
			m_iCurrLexeme ++;
			return m_delimeters [m_pArrIndex [m_iCurrLexeme - 1].j];
		}
		//
		// Keyword
		//
		case 1:
		{
			m_iCurrLexeme ++;
			return m_keywords [m_pArrIndex [m_iCurrLexeme - 1].j];
		}
		default: return _TEXT ("");
	}
}

MpString
MpLexer::getLexeme (const long index) const
{
	if ( index >= MP_ARR_LEN )
		return _TEXT ("");

	switch (m_pArrIndex [index].i)
	{
		//
		// Number
		//
		case 3:
		{
			MpStringStream ss;
			ss << m_pArrNumber [m_pArrIndex [index].j].number;
			return ss.str ();
		}
		//
		// Identifier
		//
		case 4:
		{
			return m_pArrID [m_pArrIndex [index].j].id;
		}
		//
		// Delimiter
		//
		case 2:
		{
			return m_delimeters [m_pArrIndex [index].j];
		}
		//
		// Keyword
		//
		case 1:
		{
			return m_keywords [m_pArrIndex [index].j];
		}
		default: return _TEXT ("");
	}
}

void
MpLexer::setToBegin ()
{
	m_bZeroIndex = true;
}

unsigned int
MpLexer::getCurrentLexemeIndex () const
{
	return (m_iCurrLexeme - 1);
}

MpString
MpLexer::getKeyword (int type) const
{
	if ((type >= 0) && (type < MP_ARR_LEN))
		return m_keywords [type];

	return _TEXT ("");
}

MpString
MpLexer::getDelimiter (int type) const
{
	if ((type >= 0) && (type < MP_ARR_LEN))
		return m_delimeters [type];

	return _TEXT ("");
}
