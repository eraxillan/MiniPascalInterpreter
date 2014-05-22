/**
 * @file
 * @brief The interface of lexical analyzer
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#ifndef __MINIPASCAL_LEXER_H
#define __MINIPASCAL_LEXER_H

#include <Poco/Util/LayeredConfiguration.h>

#include "types.h"

namespace MiniPascal
{
	//
	// The length of lexeme, id, number and keyword arrays
	//
	const int MP_ARR_LEN = 1000;

	//
	// Number
	//
	struct MpNumLexeme
	{
		int number;
		int count;
	};

	//
	// Identifier (ID)
	//
	struct MpIdLexeme
	{
		std::string id;
		int    count;
	};

	//
	// 3D index for lexemes
	//
	struct MpIndexLexeme
	{
		int i;
		int j;
		int k;
	};

	//
	// Program keywords such as if/else
	//
	enum
	{
		KEYWORD_PROGRAM = 0,
		KEYWORD_VAR     = 1,
		KEYWORD_INT     = 2,
		KEYWORD_BOOL    = 3,
		KEYWORD_BEGIN   = 4,
		KEYWORD_END     = 5,
		KEYWORD_IF      = 6,
		KEYWORD_THEN    = 7,
		KEYWORD_ELSE    = 8,
		KEYWORD_WHILE   = 9,
		KEYWORD_DO      = 10,
		KEYWORD_READ    = 11,
		KEYWORD_WRITE   = 12,
		KEYWORD_TRUE    = 13,
		KEYWORD_FALSE   = 14,
		KEYWORD_AND     = 15,
		KEYWORD_OR      = 16,
		KEYWORD_NOT     = 17,
		KEYWORD_UN      = 18
	};

	//
	// Delimeters such as comma
	//
	enum
	{
		DELIM_OPERATOR_END    = 0,
		DELIM_PROGRAM_END     = 1,
		DELIM_COMMA           = 2,
		DELIM_TYPE            = 3,
		DELIM_ASSUME          = 4,
		DELIM_OPEN_BRACKET    = 5,
		DELIM_CLOSE_BRACKET   = 6,
		DELIM_PLUS            = 7,
		DELIM_MINUS           = 8,
		DELIM_MUL             = 9,
		DELIM_DIV             = 10,
		DELIM_EQUAL           = 11,
		DELIM_NOT_EQUAL       = 12,
		DELIM_MORE            = 13,
		DELIM_MORE_OR_EQUAL   = 14,
		DELIM_LESSER          = 15,
		DELIM_LESSER_OR_EQUAL = 16
	};

	///////////////////////////////////////////////////////////////////////////////////////////////

	/**
	  * @brief Simple lexical analyzer: split program source to the table of lexemes
	  */
	class MpLexer
	{
		/**
	      * @brief Known keywords list
	      */
		std::vector<std::string> m_keywords;

		/**
	      * @brief Known delimeters list
	      */
		std::vector<std::string> m_delimeters;

		/**
	      * @brief Known Single line comment begin operators
	      */
		std::vector<std::string> m_slComments;

		/**
	      * @brief Known multiline comment begin and end symbols
	      * @note This list must have even length
	      */
		std::vector<std::string> m_mlComments;

		/**
	      * @brief Integer numbers found in source
	      */
		MpNumLexeme* m_pArrNumber;

		/**
	      * @brief Identifiers found in source
	      */
		MpIdLexeme* m_pArrID;

		/**
	      * @brief Indeces of lexemes (k1, k2, k3)
	      */
		MpIndexLexeme* m_pArrIndex;

		/**
	      * @brief Total number of detected lexemes
	      */
		size_t m_nLexemCount;

		/**
	      * @brief Whether @a getNextLexeme should lexeme index to null
	      */
		bool m_bZeroIndex;

		/**
	      * @brief Current lexeme index
	      */
		size_t m_curr_lexeme_idx;

		/**
		  * @brief Logger stream
		  */
		Poco::LogStream& m_logstream;

		/**
	      * @brief Skip single-line and multi-line comments
	      */
		bool skipComments (std::istream& _f, std::string& _line, long& _line_index);

		/**
	      * @brief Check whether specified lexeme is a number
	      */
		bool isNumber (const std::string& _token, int& _num);

		/**
	      * @brief Check whether specified lexeme is a keyword
	      */
		bool isKeyword (const std::string& _token, int& _index) const;

		/**
	      * @brief Check whether specified lexeme is a delimeter
	      */
		bool isDelimiter (const std::string& _token, int& _index);

		/**
	      * @brief Find token type and write it to the proper table
	      */
		bool writeToTable (const std::string& _token, const long& _line_index);

	public:
		explicit MpLexer (Poco::LogStream& _logstream);
		~MpLexer ();

		/**
	      * @brief Load keywords and delimeters from specified text configuration file
	      */
		bool loadConfig (Poco::Util::LayeredConfiguration& _config);

		/**
	      * @brief Extract lexemes from specified source code file
	      */
		bool loadFile (const std::string& _name);

		/**
	      * @brief Save lexemes to specified file
	      * @note Useful for debugging purposes
	      */
		bool saveLexemeFile (const std::string& _name) const;

		/**
	      * @brief Return next lexeme in lexeme table or "", if EOF found
	      */
		std::string getNextLexeme (long* _line_index);
		
		/**
	  * @brief Return specified lexeme from lexeme table
	  */
		std::string getLexeme (const long _index) const;
		
		/**
	      * @brief Set current lexeme to program first.
	      */
		void setToBegin ();
		
		/**
	      * @brief Return current lexeme index
	      */
		size_t getCurrentLexemeIndex () const;

		/**
	      * @brief Return specified keyword
	      */
		std::string getKeyword (int _type) const;
		
		/**
	      * @brief Return specified delimiter
	      */
		std::string getDelimiter (int _type) const;
	};
}

#endif // __MINIPASCAL_LEXER_H