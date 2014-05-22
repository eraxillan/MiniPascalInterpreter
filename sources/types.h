/**
 * @file
 * @brief Helpers for compiling in both Unicode and ANSI modes
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */

#ifndef __MINIPASCAL_TYPES_H
#define __MINIPASCAL_TYPES_H

#define _CRT_SECURE_NO_WARNINGS

//
// C++ STL headers
//
#include <algorithm>
#include <functional>

#include <string>

#include <sstream>

#include <vector>
#include <stack>
#include <set>
#include <map>

//
// POCO headers
//

// String operations
#include <Poco/UTF8String.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Unicode.h>
#include <Poco/UnicodeConverter.h>
#include <Poco/TextEncoding.h>

// Input/output
#include <Poco/Path.h>
#include <Poco/FileStream.h>

// Logging
#include <Poco/Logger.h>
#include <Poco/LogStream.h>
#ifdef _WIN32
#include <Poco/WindowsConsoleChannel.h>
#else
#include <Poco/ConsoleChannel.h>
#endif

//
// Check whether Poco was compiled with UTF-8 encoding support
//
#ifdef _WIN32
#ifndef POCO_WIN32_UTF8
#error "MiniPascalInterpreter will work only with UTF-8 encoding"
#endif // ! POCO_WIN32_UTF8 
#endif

namespace MiniPascal
{
	struct MpVariable;
	struct MpOpTypes;

	/**
	  * @brief Set the console window title
	  */
	inline void setConsoleTitle (const std::string& _text)
	{
#ifdef _WIN32
#ifdef _UNICODE
		std::wstring utext;
		Poco::UnicodeConverter::toUTF16 (_text, utext);
		SetConsoleTitleW (utext.data ());
#else
		SetConsoleTitleA (text);
#endif // _UNICODE
#else
		// TODO: test this under Unix terminals
		printf ("%c]0;%s%c", '\033', _text.data (), '\007');
#endif // _WIN32
	}

	//
    // Complex STL map and set types for lexer, parser, polir
	//
	typedef std::map <std::string, MpVariable, std::less <std::string> > MpVariableMap;
	typedef std::set < char, std::less <char> > MpDigitsSet;
	typedef MpDigitsSet MpLettersSet;
	typedef std::set < std::string, std::less <std::string> > MpStringsSet;
	typedef std::map < std::string, std::string, std::less <std::string> > MpStringsDict;
	typedef std::map < std::string, MpOpTypes, std::less <std::string> > MpTypesDict;

	/**
	  * @struct MpVariable
	  * @brief MiniPascal @a number variable storage structure
	  * @todo Support float types
	  * @var MpVariable::type
	  * Variable data type
	  * @var MpVariable::value
	  * Variable value
	  */
	struct MpVariable
	{
		std::string type;
		int value;
	};

	/**
      * @struct MpOpTypes
      * @brief Operand types structure for parser: first, second, result
	  * @var MpOpTypes::type1
	  * First operand data type
	  * @var MpOpTypes::type2
	  * Second operand data type
	  * @var MpOpTypes::equal
	  * Whether @a type1 and @a type2 must be the same
	  * @var MpOpTypes::typeResult
	  * Result data type
	  */
	struct MpOpTypes
	{
		std::string type1;
		std::string type2;
		bool equal;
		std::string typeResult;
	};
}

#endif // __MINIPASCAL_TYPES_H
