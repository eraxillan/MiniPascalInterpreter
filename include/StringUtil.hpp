/**
  * @file
  * @brief Helpers for compiling in both Unicode and ANSI modes
  * @author Alexander Kamyshnikov <axill777@gmail.com>
  */

#ifndef __MINIPASCAL_STRING_UTIL_HPP
#define __MINIPASCAL_STRING_UTIL_HPP

//
// C++ STL headers
//
#include <string>

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
#include <Poco/NumberFormatter.h>
#include <Poco/NumberParser.h>

// Logging
#include <Poco/LogStream.h>

namespace MiniPascal
{
	/**
	  * @brief Check whether STL string @a _str is a number and write it's value into the @a _num
	  */
	inline bool
	stringIsInt (const std::string& _str, int& _num)
	{
		return Poco::NumberParser::tryParse (_str, _num);
	}

	/**
	  * @brief Convert STL string @a _str to the integer number
	  * @return Null if @a _str is not a number
	  */
	inline int
	stringToInt (Poco::LogStream& _ls, const std::string& _str)
	{
		int value = 0;

		try
		{
			value = Poco::NumberParser::parse (_str);
		}
		catch (Poco::SyntaxException sex)
		{
			_ls.error () << "Polir, stringToInt function error: " << sex.message () << std::endl;
		}

		return value;
	}

	/**
	  * @brief Converts integer number to the STL string
	  */
	inline std::string
	intToString (int _num)
	{
		return Poco::NumberFormatter::format (_num);
	}
}

#endif // __MINIPASCAL_STRING_UTIL_HPP
