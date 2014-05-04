/**
 * @file
 * @brief The interface of POLIR converter and interpreter
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */

#ifndef __MINIPASCAL_POLIR_H
#define __MINIPASCAL_POLIR_H

#include "lexer.h"
#include "parser.h"

namespace MiniPascal
{
	/**
	  * @brief Convert lexemes to POLIR and interpret them
	  */
	class MpPolir
	{
		typedef std::map <MpString, unsigned char, std::less<MpString>> PriorityMap;

		/**
		  * @brief Operators stack
		  */
		std::stack <MpString> m_opStack;

		/**
		  * @brief Operators priorities
		  */
		PriorityMap m_opPriors;

		/**
		  * @brief Lexer object with ready to use lexemes list
		  */
		MpLexer* m_lexer;

		/**
		  * @brief Parser object with ready to use variable and operator tables
		  */
		MpParser* m_parser;

		/**
		  * @brief Declared in source variables
		  */
		MpVariableMap m_vars;

		/**
		  * @brief Ready to interpret POLIR record
		  */
		std::vector <MpString> m_polirExpr;

		/**
		  * @brief Return operation priority (255 if not found)
		  */
		int priority (MpString op) const;

		/**
		  * @brief Converts any valid simple expression to POLIR.
		  */
		void convert (const MpString& lexeme);

		/**
		  * @brief Use lexer and parser data
		  */
		void connectLexer (MpLexer* pLexer);
		void connectParser (MpParser* pParser);

	public:
		explicit MpPolir (MpLexer* pLex, MpParser* pPsr);

		/**
		  * @brief Convert expression to POLIR 
		  * @note Uses Edsger Wybe Dijkstra algorithm
		  */
		void convertExpression (const MpString& exp);

		/**
		  * @brief Convert operation to POLIR 
		  * @note Uses Edsger Wybe Dijkstra algorithm
		  */
		void convertOperation (const MpString& op);

		/**
		  * @brief Convert expression from current lexeme up to ";"
		  * @note Uses getNextLexeme()
		  */
		void convertExpression (const MpString& startL, bool* isConst);

		/**
		  * @brief Converts program from "begin" token up to "end", "." in postfix notation
		  */
		void convertProgram ();

		/**
		  * @brief Interpret converted program
		  */
		void executeProgram ();

		/**
		  * @brief Writes ready to interpret POLIR to specified file
		  * @note Useful for debugging purposes
		  */
		bool saveToFile (const MpChar* fileName);
	};
}

#endif // __MINIPASCAL_POLIR_H
