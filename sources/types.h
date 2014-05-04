/**
 * @file
 * @brief Helpers for compiling in both Unicode and ANSI modes
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */

#ifndef __MINIPASCAL_TYPES_H
#define __MINIPASCAL_TYPES_H

#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <functional>

#include <string>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <stack>
#include <set>
#include <map>

#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

namespace MiniPascal
{
	struct MpVariable;
	struct MpOpTypes;

	//
    // Character types
    // TODO: in GCC wchar_t has 4 bytes size, but in Visual C++ only 2. We should check it!
	//
	typedef char    MpCharA;
	typedef wchar_t MpCharU;

	//
    // C "string" functions: copy and character type determination
	//
    #define MpIsAlNumA isalnum
    #define MpIsAlNumU iswalnum
	#define MpIsSpaceA isspace
	#define MpIsSpaceU iswspace
    #define MpIsAlU iswalpha
    #define MpIsAlA isalpha
    #define MpIsDigitU iswdigit
    #define MpIsDigitA isdigit
    #define MpStrCpyU wcscpy
    #define MpStrCpyA strcpy
	#define MpToLowerU towlower
	#define MpToLowerA tolower

	//
    // C++ STL string types and vector of them
	//
	typedef std::wstring MpStringU;
	typedef std::vector<MpStringU> MpStringListU;
	typedef std::string MpStringA;
	typedef std::vector<MpStringA> MpStringListA;

	//
    // C++ STL streams, file and standard
	//
	typedef std::ostream MpOutputStream;
    typedef std::wstringstream MpStringStreamU;
    typedef std::stringstream MpStringStreamA;
    typedef std::ifstream MpInputFileStreamA;
    typedef std::wifstream MpInputFileStreamU;
    typedef std::ofstream MpOutputFileStreamA;
    typedef std::wofstream MpOutputFileStreamU;
    #define MpCinA std::cin
    #define MpCinU std::wcin
    #define MpCoutA std::cout
    #define MpCoutU std::wcout

	//
    // Constants for INI file parser
	//
    const char MP_SECTION_BEGIN_A = '[';
    const wchar_t MP_SECTION_BEGIN_U = L'[';

	//
    // Type selection depends on selected character encoding
	//
#ifdef _UNICODE
#ifndef _TEXT
    #define _TEXT(x) L ## x
#endif

    #define MpIsDigit MpIsDigitU
    #define MpIsAl MpIsAlU
    #define MpIsAlNum MpIsAlNumU
	#define MpIsSpace MpIsSpaceU
    #define MpStrCpy MpStrCpyU
	#define MpToLower MpToLowerU

	typedef MpCharU MpChar;
	typedef MpStringU MpString;
	typedef MpStringListU MpStringList;

    typedef MpStringStreamU MpStringStream;
    #define MpCin MpCinU
    #define MpCout MpCoutU
	typedef MpInputFileStreamU MpInputFileStream;
    typedef MpOutputFileStreamU MpOutputFileStream;

    #define MP_SECTION_BEGIN MP_SECTION_BEGIN_U
#else
	#ifndef _TEXT
		#define _TEXT(x) x
	#endif

    #define MpIsDigit MpIsDigitA
    #define MpIsAl MpIsAlA
    #define MpIsAlNum MpIsAlNumA
	#define MpIsSpace MpIsSpaceA
    #define MpStrCpy MpStrCpyA
	#define MpToLower MpToLowerA

	typedef MpCharA MpChar;
	typedef MpStringA MpString;
	typedef MpStringListA MpStringList;

    typedef MpStringStreamA MpStringStream;
    #define MpCin MpCinA
    #define MpCout MpCoutA
	typedef MpInputFileStreamA MpInputFileStream;
    typedef MpOutputFileStreamA MpOutputFileStream;

    #define MP_SECTION_BEGIN MP_SECTION_BEGIN_A
#endif // _UNICODE

#ifdef _WIN32
	#define MP_COLOR_BLUEGREEN (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
	#define MP_COLOR_REDGREEN (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
	#define MP_COLOR_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#else
	#define MP_COLOR_BLUEGREEN 0
	#define MP_COLOR_REDGREEN 0
	#define MP_COLOR_RED 0
#endif // _WIN32

	//
    // Console text colors definition
	//
	#define MP_COLOR_NORMAL  MP_COLOR_BLUEGREEN
	#define MP_COLOR_WARNING MP_COLOR_REDGREEN
	#define MP_COLOR_ERROR   MP_COLOR_RED

	///////////////////////////////////////////////////////////////////////////////////////////////

	//
    // Console and OS helper functions
    // TODO: Unix/gcc implementation
	//

	inline void toLower (MpString& _str)
	{
		std::transform (_str.begin (), _str.end (), _str.begin (),
						std::bind (MpToLower, std::placeholders::_1));
	}

	/**
	  * @brief Set the console window title
	  * @note Currently working under Windows only
	  */
    extern void setConsoleTitle (const MpChar* _title);

	/**
	  * @brief  Set the foreground text color
	  * @note Currently working under Windows only
	  * @param[in] _cl New foreground text color in Windows console color format: FOREGROUND_*
	  */
    extern void setTextColor (unsigned short _cl);

	/**
	  * @brief Forces console window to stop work and wait for any key have been pressed.
	  * @note Currently working under Windows only
	  */
    extern void systemPause ();

	//
    // Complex STL map and set types for lexer, parser, polir
	//
	typedef std::map <MpString, MpVariable, std::less <MpString> > MpVariableMap;
	typedef std::set < MpChar, std::less <MpChar> > MpDigitsSet;
	typedef MpDigitsSet MpLettersSet;
	typedef std::set < MpString, std::less <MpString> > MpStringsSet;
	typedef std::map < MpString, MpString, std::less <MpString> > MpStringsDict;
	typedef std::map < MpString, MpOpTypes, std::less <MpString> > MpTypesDict;

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
		MpString type;
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
		MpString type1;
		MpString type2;
		bool equal;
		MpString typeResult;
	};
}

#endif // __MINIPASCAL_TYPES_H
