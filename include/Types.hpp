/**
  * @file
  * @brief MiniPascal interpreter complex data types
  * @author Alexander Kamyshnikov <axill777@gmail.com>
  */

#ifndef __MINIPASCAL_TYPES_HPP
#define __MINIPASCAL_TYPES_HPP

//
// Eliminate "no secure" warnings under MSVC
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

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

// Memory
#include <Poco/SingletonHolder.h>

// Input/output
#include <Poco/Path.h>
#include <Poco/FileStream.h>

// Logging
#include <Poco/Logger.h>
#include <Poco/LogStream.h>
#include <Poco/FormattingChannel.h>
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
#error "MiniPascalInterpreter will work only with UTF-8 encoding! Please compile PoCo with POCO_WIN32_UTF8 defined"
#endif // ! POCO_WIN32_UTF8 
#endif

namespace MiniPascal
{
	struct MpVariable;
	struct MpOpTypes;

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

#endif // __MINIPASCAL_TYPES_HPP
