/**
 * @file
 * @brief The interface of syntax analyzer
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#ifndef __MINIPASCAL_PARSER_H
#define __MINIPASCAL_PARSER_H

#include "types.h"
#include "lexer.h"

namespace MiniPascal
{
	// FIXME: replace parsing procedure names to more clear ones :)

	/**
	  * @brief Simple syntax analyzer: split lexemes to the sequence of operations
	  * under variables and constants
	  * @note This is recursive descent parser
	  */
	class MpParser
	{
		/**
	      * @brief Lexical analyzer object
	      */
		MpLexer* m_lexer;

		/**
	      * @brief Current lexeme
	      */
		std::string m_sCurrLexeme;

		/**
	      * @brief Current line index
	      */
		long     m_iCurrLine;

		/**
	      * @brief 0...9 set
	      */
		MpDigitsSet  m_digits;

		/**
	      * @brief  a...z set
	      */
		MpLettersSet m_letters;

		/**
	      * @brief Expression operands and their types stack
	      */
		std::stack <std::string> m_exprOpType;

		/**
	      * @brief Valid data types
	      */
		// TODO: add valid types definitions to config file.
		//MpStringsSet m_validTypes;

		/**
	      * @brief Already used in MP-program variables
		  * @note Used for "unreferenced local var" warning generation
	      */
		MpStringsSet m_usedVars;
		
		/**
	      * @brief Prints syntax error and shutdown the program.
	      */
		void ERR (const std::string& text);
		
		/**
	      * @brief Prints semantic error and shutdown the program
	      */
		void ERR2 (const std::string& text);
		
		/**
	      * @brief Move to the next lexeme
	      */
		void GC ();
		
		/**
	      * @brief Shows current lexeme and text "name"
		  * @note Useful for debugging purposes
	      */
		void INFO (const std::string& name) const;

		/**
	      * @brief "program" keyword handler
	      */
		void P ();
		
		/**
	      * @brief "var" keyword handler
	      */
		void D1 ();
		
		/**
	      * @brief Reads variables and their types to the internal map
	      */
		void D2 ();
		
		/**
	      * @brief Code block handler
	      */
		void B (bool main);
		
		/**
	      * @brief Main program text handler
	      */
		void S ();
		
		/**
	      * @brief Compare operators (<, <=, >, >=, =, <>) handler
	      */
		void E ();
		
		/**
	      * @brief Sum operators: +, -, logical "or" handler
	      */
		void E1 ();
		
		/**
	      * @brief Multiply operators *, /, and logical "and" handler
	      */
		void T ();
		
		/**
	      * @brief "not", "un" operators and brackets "(", ")" handler
	      */
		void F ();
		
		/**
	      * @brief Boolean constant handler (true/false)
	      */
		void L ();
		
		/**
	      * @brief Identifier handler
	      */
		void I ();
		
		/**
	      * @brief Number handler
		  * @todo Only integers are supported now, one need to add float support
	      */
		void N ();

		/**
	      * @brief Check expression variables for type mismatch
		  * @note Shutdown program on failure
	      */
		void checkTypes ();

		// FIXME: remove public access for those vars
	public:
		/**
	      * @brief Declared variables array, name -> type.
	      */
		MpStringsDict m_validVars;

		/**
	      * @brief Operands and result types for binary operators.
	      */
		MpTypesDict   m_opTypes;

		/**
		  * @brief Logger stream
		  */
		Poco::LogStream& m_logstream;

	public:
		explicit MpParser (MpLexer* _lexer, Poco::LogStream& _logstream);

		/**
	      * @brief Load lexeme table using lexer object.
	      */
		bool loadLexemeTable (MpLexer* _lexer);

		/**
	      * @brief Parse specified in lexer file.
	      */
		void parse ();
	};
}

#endif // __MINIPASCAL_PARSER_H