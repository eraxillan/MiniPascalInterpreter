/**
 * @file
 * @brief The interface of lexical analyzer
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "config_file.h"

#ifndef __MINIPASCAL_LEXER_H
#define __MINIPASCAL_LEXER_H

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
		MpString id;
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
		MpStringList m_keywords;

		/**
	      * @brief Known delimeters list
	      */
		MpStringList m_delimeters;

		/**
	      * @brief Known Single line comment begin operators
	      */
		MpStringList m_slComments;

		/**
	      * @brief Known multiline comment begin and end symbols
	      * @note This list must have even length
	      */
		MpStringList m_mlComments;

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
		size_t m_iCurrLexeme;

		/**
	      * @brief Skip single-line and multi-line comments
	      */
		bool skipComments (MpInputFileStream& f, MpString& line, long& lineIndex);

		/**
	      * @brief Check whether specified lexeme is a number
	      */
		bool isNumber (const MpString& token, int& num);

		/**
	      * @brief Check whether specified lexeme is a keyword
	      */
		bool isKeyword (const MpString& token, int& index) const;

		/**
	      * @brief Check whether specified lexeme is a delimeter
	      */
		bool isDelimiter (const MpString& token, int& index);

		/**
	      * @brief Find token type and write it to the proper table
	      */
		bool writeToTable (const MpString& token, const long& lineIndex);

	public:
		explicit MpLexer ();
		~MpLexer ();

	public:
		/**
	      * @brief Load keywords and delimeters from specified text configuration file
	      */
		bool         loadConfig (const MpChar * name);

		/**
	      * @brief Extract lexemes from specified source code file
	      */
		bool         loadFile (const MpChar * name);

		/**
	      * @brief Save lexemes to specified file
	      * @note Useful for debugging purposes
	      */
		bool         saveLexemeFile (const MpChar * name) const;

		/**
	      * @brief Return next lexeme in lexeme table or "", if EOF found
	      */
		MpString     getNextLexeme (long * lineIndex);
		
		/**
	  * @brief Return specified lexeme from lexeme table
	  */
		MpString     getLexeme (const long index) const;
		
		/**
	      * @brief Set current lexeme to program first.
	      */
		void         setToBegin ();
		
		/**
	      * @brief Return current lexeme index
	      */
		size_t       getCurrentLexemeIndex () const;

		/**
	      * @brief Return specified keyword
	      */
		MpString     getKeyword (int type) const;
		
		/**
	      * @brief Return specified delimiter
	      */
		MpString     getDelimiter (int type) const;
	};
}

#endif // __MINIPASCAL_LEXER_H