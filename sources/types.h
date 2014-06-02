/**
 * @file
 * @brief Helpers for compiling in both Unicode and ANSI modes
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */

#ifndef __MINIPASCAL_TYPES_H
#define __MINIPASCAL_TYPES_H

//
// Eliminate "no secure" warnings under MSVC
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
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

// String operations
#include <Poco/UTF8String.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Unicode.h>
#include <Poco/UnicodeConverter.h>
#include <Poco/TextEncoding.h>
#include <Poco/NumberFormatter.h>
#include <Poco/NumberParser.h>

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
#error "MiniPascalInterpreter will work only with UTF-8 encoding"
#endif // ! POCO_WIN32_UTF8 
#endif

//
// Make various max() functions compilable;
// (otherwise it will be overrided by the "max" macro from C stdlib)
//
#undef max

namespace MiniPascal
{
	struct MpVariable;
	struct MpOpTypes;

	class UnicodeConsole
	{
		//UnicodeConsole ();

	public:
		// FIXME: Poco SingletonHolder create object using it's constructor;
		// it break Singleton pattern "purity", i.e. client can create it's own instance of class
		UnicodeConsole ()
		{
#ifdef _WIN32
			//
			// Change the console mode to the UTF-16 (it is OEM by default, e.g. ibm-866 for Russian OS)
			//
			fflush (stdout);
			fflush (stdin);
			_setmode (_fileno (stdout), _O_U16TEXT);
			_setmode (_fileno (stdin), _O_U16TEXT);
#endif
		}

		static UnicodeConsole& instance ()
		{
			static Poco::SingletonHolder<UnicodeConsole> sh;
			return (*sh.get ());
		}

		inline void
		writeLine (const std::string& _value)
		{
#ifdef _WIN32
			//
			// NOTE: Under Windows we have to convert our UTF-8 string to UTF-16 encoded one
			//
			std::wstring uvalue;
			Poco::UnicodeConverter::toUTF16 (_value, uvalue);

			std::wcout << uvalue << std::endl;
#else
			//
			// Under Unix we just write the string value "as is"
			//
			std::cout << _value << std::endl;
#endif
		}

		/**
		  * @brief Read the string value from @a std::cin and return it as UTF-8 encoded
		  * @note Under Windows function assume that console support UTF-16
		  */
		inline std::string
		readLine ()
		{
			std::string value;
			
			//
			// NOTE: Under Windows we have to convert user entered UTF-16 string to UTF-8 encoded one
			//
#ifdef _WIN32
			std::wstring uvalue;
			std::wcin >> uvalue;
			
			Poco::UnicodeConverter::toUTF8 (uvalue, value);
#else
			//
			// Under Unix we just read the string value "as is"
			//
			std::cin >> value;
#endif

			return value;
		}

		/**
		* @brief Set the console window title
		*/
		inline void
		setTitle (const std::string& _text)
		{
#ifdef _WIN32
			std::wstring utext;
			Poco::UnicodeConverter::toUTF16 (_text, utext);
			SetConsoleTitleW (utext.data ());
#else
			// TODO: test this under Unix terminals
			printf ("%c]0;%s%c", '\033', _text.data (), '\007');
#endif // _WIN32
		}

		inline void
		pause ()
		{
			std::wcout << L"Press any key to continue..." << std::endl;
			std::wcin.clear ();
			std::wcin.sync ();
			std::wcin.get ();
		}
	};

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
