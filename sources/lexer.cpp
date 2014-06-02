/**
 * @file
 * @brief The implementation of lexical analyzer
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   
#include <Poco/StringTokenizer.h>

#include "types.h"
#include "lexer.h"

using namespace MiniPascal;

bool
MpLexer::skipComments (std::istream& f, std::string& line, long& lineIndex)
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
		if ((iR >= (int) m_mlComments [1].length ()) && (iR != -1))
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
				std::getline (f, line);
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
				// Invalid comment detected, close brace is absent
				//
				m_logstream.error () << "[" << lineIndex << "]" << " LEXER ERROR: " << "Invalid comment - close symbol not found!" << std::endl;
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
MpLexer::isKeyword (const std::string& _token, int& _index) const
{
	for (int i = 0; i < m_keywords.size (); i ++)
	{
		if (!Poco::UTF8::icompare (_token, m_keywords [i]))
		{
			_index = i;
			return true;
		}
	}

	return false;
}

bool
MpLexer::isDelimiter (const std::string& _token, int& _index)
{
	int i = 0;
	for ( ; i < m_delimeters.size (); i ++)
	{
		if (!Poco::UTF8::icompare (_token, m_delimeters [i]))
		{
			_index = i;
			return true;
		}
	}

	return false;
}

bool
MpLexer::writeToTable (const std::string& _token, const long& _line_index)
{
	if (_token.empty ())
		return false;

	//DEBUG:
	//cout << "WTT, Token = " << token << endl;

	//
	// Token is number (table 3)
	//
	int num = 0;
	if (stringIsInt (_token, num))
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
					(m_pArrNumber [(i + l*l + l + 1) % MP_ARR_LEN].number != 0))
				{
					l++;
				}

				m_pArrNumber [(i + l*l + l + 1) % MP_ARR_LEN].number = num;
				m_pArrNumber [(i + l*l + l + 1) % MP_ARR_LEN].count ++;
				i += l;
			}
		}

		//
		// Add to index table
		//
		m_pArrIndex [m_nLexemCount].i = 3;           // Number of table
		m_pArrIndex [m_nLexemCount].j = i;           // Table index
		m_pArrIndex [m_nLexemCount].k = _line_index; // Lexeme line index
		m_nLexemCount ++;
		if (m_nLexemCount >= MP_ARR_LEN)
		{
			m_logstream.error () << "LEXER ERROR: Too many lexems!" << std::endl;
			return false;
		}
		return true;
	}

	//
	// Token is delimiter (table 2)
	//
	int index = 0;
	if (isDelimiter (_token, index))
	{
		//
		// Add to index table
		//
		m_pArrIndex [m_nLexemCount].i = 2;
		m_pArrIndex [m_nLexemCount].j = index;
		m_pArrIndex [m_nLexemCount].k = _line_index;
		m_nLexemCount ++;
		if (m_nLexemCount >= MP_ARR_LEN)
		{
			m_logstream.error () << "LEXER ERROR: Too many lexems!" << std::endl;
			return false;
		}
		return true;
	}

	//
	// Token is keyword (table 1)
	//
	index = 0;
	if (isKeyword (_token, index))
	{
		//
		// Add to index table
		//
		m_pArrIndex [m_nLexemCount].i = 1;
		m_pArrIndex [m_nLexemCount].j = index;
		m_pArrIndex [m_nLexemCount].k = _line_index;
		m_nLexemCount ++;
		if (m_nLexemCount >= MP_ARR_LEN)
		{
			m_logstream.error () << "LEXER ERROR: Too many lexems!" << std::endl;
			return false;
		}
		return true;
	}

	//
	// Else token is ID (table 4)
	//
	long n = _token.length ();

	//
	// Check token for errors
	//
	if (isdigit (_token [0]))
	{
		m_logstream.error () << "[" << _line_index << "]" << " LEXER ERROR: " << "invalid ID " << _token << "." << std::endl;
		return false;
	}
	for (long j = 0L; j < n; j ++)
	{
		if (!Poco::Unicode::isAlpha (_token [j]) && !Poco::Unicode::isDigit (_token [j]))
		{
			m_logstream.error () << "[" << _line_index << "]" << " LEXER ERROR: " << "invalid char " << _token [j] << "." << std::endl;
			return false;
		}
	}

	//
	// Evaluate the hash function value
	//
	long i = (long) ((_token [0] + _token [n / 2] + _token [n - 1]) % MP_ARR_LEN);

	if (!m_pArrID [i].id.length ())
	{
		//
		// The table cell is empty
		//
		m_pArrID [i].id = _token;
		m_pArrID [i].count ++;
	}
	else
	{
		if (!Poco::UTF8::icompare (m_pArrID [i].id, _token))
			m_pArrID [i].count ++;
		else
		{
			//
			// Handle the collision
			//
			long l = 1;
			while ((Poco::UTF8::icompare (m_pArrID [(i + l) % MP_ARR_LEN].id, _token) != 0) &&
				(!m_pArrID [(i + l) % MP_ARR_LEN].id.empty ()))
				l++;

			m_pArrID [(i + l) % MP_ARR_LEN].id = _token;
			m_pArrID [(i + l) % MP_ARR_LEN].count ++;
			i += l;
		}
	}

	//
	// Add new lexeme to the index table
	//
	m_pArrIndex [m_nLexemCount].i = 4;
	m_pArrIndex [m_nLexemCount].j = i;
	m_pArrIndex [m_nLexemCount].k = _line_index;
	m_nLexemCount ++;
	if (m_nLexemCount >= MP_ARR_LEN)
	{
		m_logstream.error () << "[" << _line_index << "]" << " LEXER ERROR: " << "Too many lexems!" << std::endl;
		return false;
	}

	return true;
}

MpLexer::MpLexer (Poco::LogStream& _logstream) : m_logstream (_logstream)
{
	m_curr_lexeme_idx = 0;

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
		m_pArrID [i].id.clear ();
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
MpLexer::loadConfig (Poco::Util::LayeredConfiguration& _config)
{
	//
	// Read keywords list from the configuration file and validate it
	//
	auto keywords_str = _config.getString ("Lexer.Keywords");
	Poco::StringTokenizer st_keywords (keywords_str, " ", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
	m_keywords.assign (st_keywords.begin (), st_keywords.end ());
	while (m_keywords.size () < MP_ARR_LEN)
		m_keywords.push_back (std::string ());
	if (m_keywords.empty ())
	{
		m_logstream.error () << "LEXER I/O ERROR: Keyword list from configuration file is empty!" 
			<< std::endl << "Unable to continue parsing" << std::endl;
		return false;
	}

	auto delim_str = _config.getString ("Lexer.Delimiters");
	Poco::StringTokenizer st_delim (delim_str, " ", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
	m_delimeters.assign (st_delim.begin (), st_delim.end ());
	while (m_delimeters.size () < MP_ARR_LEN)
		m_delimeters.push_back (std::string ());
	if (m_delimeters.empty ())
	{
		m_logstream.error () << "LEXER I/O ERROR: Delimeter list from config is empty!" 
			<< std::endl << "Unable to continue parsing" << std::endl;
		return false;
	}

	auto singleline_comment_str = _config.getString ("Lexer.Singleline_comment");
	Poco::StringTokenizer st_sl_comment (singleline_comment_str, " ", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
	m_slComments.assign (st_sl_comment.begin (), st_sl_comment.end ());
	while (m_slComments.size () < MP_ARR_LEN)
		m_slComments.push_back (std::string ());
	if (m_slComments.empty ())
	{
		m_logstream.error () << "LEXER I/O ERROR: Keyword list from config is empty!"
			<< std::endl << "Unable to continue parsing" << std::endl;
		return false;
	}

	auto multiline_comment_str = _config.getString ("Lexer.Multiline_comment");
	Poco::StringTokenizer st_ml_comment (multiline_comment_str, " ", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
	m_mlComments.assign (st_ml_comment.begin (), st_ml_comment.end ());
	while (m_mlComments.size () < MP_ARR_LEN)
		m_mlComments.push_back (std::string ());
	if (m_mlComments.empty ())
	{
		m_logstream.error () << "LEXER I/O ERROR: Keyword list from config is empty!"
			<< std::endl << "unable to continue parsing" << std::endl;
		return false;
	}

	return true;
}

bool
MpLexer::loadFile (const std::string& _name)
{
	long lineCounter = 0L, i = 0L;

	try
	{
		//
		// Extract numbers and ID's from file
		//
		Poco::FileInputStream fi (_name);
		while (!fi.eof ())
		{
			std::string line, token, sd;
			int index = 0;

			//
			// Read the next file from source
			//
			std::getline (fi, line);
			Poco::UTF8::toLowerInPlace (line);
			lineCounter ++;
			i = 0;

			//
			// Clear line from comments
			//
			if (!skipComments (fi, line, lineCounter))
				return false;
			if (line.length () <= 1)
				continue;

			//
			// ...And from spaces
			//
			Poco::trimInPlace (line);
			if (line.empty ())
				continue;

			m_logstream.debug () << "______________________________________________" << std::endl;
			m_logstream.debug () << "Line [" << lineCounter << "]: " << line << " | " << line.length () << std::endl;

			//
			// Read the next token
			//
			i = 0;
			token.clear ();

			bool tokenFound = false, delimFound = false;
			while (i <= (int)line.length ())
			{
				if (tokenFound)
				{
					m_logstream.debug () << "Token: " << token << std::endl;
					int num = 0, index = 0;
					if (stringIsInt (token, num))
						m_logstream.debug () << "Number (3): " << num << std::endl;
					else if (isKeyword (token, index))
						m_logstream.debug () << "Keyword (1): " << token << std::endl;
					else
						m_logstream.debug () << "ID (4): " << token << std::endl;

					if (!writeToTable (token, lineCounter))
						return false;

					token.clear ();
					tokenFound = false;
				}

				if (delimFound)
				{
					m_logstream.debug () << "Delim (2): " << sd << std::endl;

					writeToTable (sd, lineCounter);
					sd.clear ();
					delimFound = false;
				}
				if (i == line.length ())
					break;

				//
				// Skip extra spaces
				//
				if (Poco::Unicode::isSpace (line [i]))
				{
					//
					// Token is ready
					//
					if (!token.empty ())
					{
						tokenFound = true;
						continue;
					}

					while (Poco::Unicode::isSpace (line [i]))
						i++;
				}

				//
				// Multi-symbol delimiter
				// NOTE: currently only two-symbol delims are supported
				//
				if (i <= (int)line.length () - 2)
				{
					sd.clear ();
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
						i++;
						i++;

						continue;
					}
				}

				//
				// One-symbol delimiter
				//
				sd.clear ();
				sd = line [i];
				if (isDelimiter (sd, index))
				{
					//
					// Token is ready
					//
					if (!token.empty ())
						tokenFound = true;

					delimFound = true;
					i++;

					continue;
				}

				if ((i + 1) == line.length ())
				{
					if (!token.empty ())
						tokenFound = true;
				}

				token += line [i];
				i++;
			} //end line cycle
		}

		fi.close ();
	}
	catch (...)
	{
		m_logstream.error () << "LEXER I/O ERROR: Could not open file " + _name + " for read" << std::endl;
		return false;
	}

	m_logstream.debug () << "LEXER INFO: No errors!" << std::endl;
	return true;
}

bool
MpLexer::saveLexemeFile (const std::string& _name) const
{
	try
	{
		Poco::FileOutputStream fLexems (_name);
		Poco::FileOutputStream fNumbers (_name + "_numbers.txt");
		Poco::FileOutputStream fIDs (_name + "_ids.txt");

		// DEBUG:
		//cout << "ID and Num Table:" << endl;

		long i = 0L;
		for (i = 0L; i < MP_ARR_LEN; i++)
		{
			if (m_pArrNumber [i].count)
			{
				// DEBUG:
				//cout << "[" << i << "] " << "Number = " << arrNumber [i].number << endl;

				fNumbers << m_pArrNumber [i].number << " " << i << std::endl;
			}
			if (m_pArrID [i].count)
			{
				// DEBUG:
				//cout << "[" << i << "] " << "ID = " << arrID [i].id << endl;

				fIDs << m_pArrID [i].id << " " << i << std::endl;
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
				fLexems << m_pArrIndex [i].i << " " << m_pArrIndex [i].j << " " << m_pArrIndex [i].k << std::endl;
			}
		}

		fLexems.close ();
		fNumbers.close ();
		fIDs.close ();
	}
	catch (...)
	{
		m_logstream.error () << "LEXER I/O ERROR: Could not open file " + _name + " in write mode" << std::endl;
		return false;
	}

	return true;
}

std::string
MpLexer::getNextLexeme (long* _line_index)
{
	if (m_bZeroIndex)
	{
		m_curr_lexeme_idx = 0;
		m_bZeroIndex = false;
	}
	if (_line_index)
		*_line_index = m_pArrIndex [m_curr_lexeme_idx].k;

	switch (m_pArrIndex [m_curr_lexeme_idx].i)
	{
		//
		// Number
		//
		case 3:
		{
			return intToString (m_pArrNumber [m_pArrIndex [m_curr_lexeme_idx++].j].number);
		}
		//
		// Identifier
		//
		case 4:
		{
			m_curr_lexeme_idx ++;
			return m_pArrID [m_pArrIndex [m_curr_lexeme_idx - 1].j].id;
		}
		//
		// Delimiter
		//
		case 2:
		{
			m_curr_lexeme_idx ++;
			return m_delimeters [m_pArrIndex [m_curr_lexeme_idx - 1].j];
		}
		//
		// Keyword
		//
		case 1:
		{
			m_curr_lexeme_idx ++;
			return m_keywords [m_pArrIndex [m_curr_lexeme_idx - 1].j];
		}
		default: return std::string ();
	}
}

std::string
MpLexer::getLexeme (const long _index) const
{
	if (_index >= MP_ARR_LEN)
		return std::string ();

	switch (m_pArrIndex [_index].i)
	{
		//
		// Number
		//
		case 3:
		{
			return intToString (m_pArrNumber [m_pArrIndex [_index].j].number);
		}
		//
		// Identifier
		//
		case 4:
		{
			return m_pArrID [m_pArrIndex [_index].j].id;
		}
		//
		// Delimiter
		//
		case 2:
		{
			return m_delimeters [m_pArrIndex [_index].j];
		}
		//
		// Keyword
		//
		case 1:
		{
			return m_keywords [m_pArrIndex [_index].j];
		}
		default: return std::string ();
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
	return (m_curr_lexeme_idx - 1);
}

std::string
MpLexer::getKeyword (int _type) const
{
	if ((_type >= 0) && (_type < MP_ARR_LEN))
		return m_keywords [_type];

	return std::string ();
}

std::string
MpLexer::getDelimiter (int _type) const
{
	if ((_type >= 0) && (_type < MP_ARR_LEN))
		return m_delimeters [_type];

	return std::string ();
}
