/**
  * @file
  * @brief The interface of POLIR converter and interpreter
  * @author Alexander Kamyshnikov <axill777@gmail.com>
  */

#ifndef __MINIPASCAL_POLIR_H
#define __MINIPASCAL_POLIR_H

#include "Lexer.hpp"
#include "Parser.hpp"

namespace MiniPascal
{
	/**
	  * @brief Convert lexemes to POLIR and interpret them
	  */
	class MpPolir
	{
		typedef std::map <std::string, unsigned char, std::less<std::string>> PriorityMap;

		/**
		  * @brief Operators stack
		  */
		std::stack <std::string> m_opStack;

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
		std::vector <std::string> m_polirExpr;

		/**
		  * @brief Logger stream
		  */
		Poco::LogStream& m_logstream;

		/**
		  * @brief Return operation priority (255 if not found)
		  */
		int priority (const std::string& op) const;

		/**
		  * @brief Converts any valid simple expression to POLIR.
		  */
		void convert (const std::string& lexeme);

		/**
		  * @brief Use lexer and parser data
		  */
		void connectLexer (MpLexer* pLexer);
		void connectParser (MpParser* pParser);

	public:
		explicit MpPolir (MpLexer* _lexer, MpParser* _parser, Poco::LogStream& _logstream);

		/**
		  * @brief Convert expression to POLIR 
		  * @note Uses Edsger Wybe Dijkstra algorithm
		  */
		void convertExpression (const std::string& exp);

		/**
		  * @brief Convert operation to POLIR 
		  * @note Uses Edsger Wybe Dijkstra algorithm
		  */
		void convertOperation (const std::string& op);

		/**
		  * @brief Convert expression from current lexeme up to ";"
		  * @note Uses getNextLexeme()
		  */
		void convertExpression (const std::string& startL, bool* isConst);

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
		bool saveToFile (const std::string& fileName);
	};
}

#endif // __MINIPASCAL_POLIR_H
