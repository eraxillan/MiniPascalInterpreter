/**
 * @file
 * @brief The interpreter entry point
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "polir.h"

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

using namespace std::placeholders;
using namespace MiniPascal;

void
MiniPascal::systemPause ()
{
#ifdef _WIN32
	system ("pause");
#else
	system("read -p \"$*\"");
#endif
}

void
MiniPascal::setConsoleTitle (const MpChar* text)
{
#ifdef _WIN32
#ifdef _UNICODE
	SetConsoleTitleW (text);
#else
	SetConsoleTitleA (text);
#endif // _UNICODE
#endif // _WIN32
}

void
MiniPascal::setTextColor (unsigned short _cl)
{
#ifdef _WIN32
	SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), _cl);
#else
	#error "Unsupported OS";
#endif // _WIN32
}

namespace
{
	/**
	  * @brief Split the command line arguments into the nice string list
	  */
	static std::vector<MpString>
	ParseCommandLine (const MpChar* _raw_cmd_line)
	{
		MpString cmd_line (_raw_cmd_line);
		enum ParserMode { InQuotes, InText, InSpace };
		ParserMode mode = InText;

		std::vector <MpString> args;
		MpString current_arg;

		for (std::size_t i = 0; i < cmd_line.length (); ++i)
		{
			//
			// Parse enquoted paths
			//
			// FIXME: handle inner quotes (e.g. ""path"")
			//
			if (cmd_line [i] == _TEXT ('"'))
			{
				if (mode != InQuotes)
				{
					mode = InQuotes;
					current_arg += _TEXT ('"');
				}
				else
				{
					mode = InText;

					current_arg += _TEXT ('"');
					args.push_back (current_arg);
					current_arg.clear ();
				}
			}
			//
			// Handle spaces
			//
			else if (MpIsSpace (cmd_line[i]))
			{
				if (mode != InQuotes)
				{
					mode = InSpace;

					if (!current_arg.empty ())
						args.push_back (current_arg);
					current_arg.clear ();
				}
				else
					current_arg += cmd_line[i];
			}
			//
			// Other symbol is a part of argument
			//
			else
			{
				if (mode == InSpace || i == cmd_line.length ()-1)
				{
					mode = InText;

					if (i == cmd_line.length ()-1)
						current_arg += cmd_line[i];

					if (!current_arg.empty ())
						args.push_back (current_arg);
					current_arg.clear ();
				}

				current_arg += cmd_line[i];
			}
		}

		return args;
	}
}

int
_tmain (int _argc, MpChar* /*argv[]*/)
{
	//
	// Set console window attributes
	//
	setTextColor (MP_COLOR_NORMAL);
	setConsoleTitle (_TEXT ("Pascal-like language compiler"));

	//
	// Parse the command line
	//
	if (_argc < 2)
	{
		MpCout << _TEXT ("ERROR: Pascal source code file not specified.") << std::endl;
		systemPause ();
		exit (0);
	}

	//
	// Parse command line options
	//
	std::vector<MpString> args = ParseCommandLine (GetCommandLine ());

	const MpString sourceFileName = args[1];
	MpCout << _TEXT ("SOURCE FILE: ") << sourceFileName << std::endl;

	bool saveLexemeFile = false;
	bool savePolirFile = false;

	std::vector<MpString>::iterator iArg = args.begin ();
	for ( ; iArg != args.end (); iArg ++)
	{
		if ((_TEXT ("--write-lexeme-file") == (*iArg)) || (_TEXT ("-l") == (*iArg)) || (_TEXT ("-L") == (*iArg)))
			saveLexemeFile = true;

		if ((_TEXT ("--write-MpPolir-file") == (*iArg)) || (_TEXT ("-p") == (*iArg)) || (_TEXT ("-P") == (*iArg)))
			savePolirFile = true;
	}

	//
	// Create and initialize the lexer object
	//
	std::unique_ptr<MpLexer> lex (new MpLexer ());
	if (!lex->loadConfig (_TEXT ("LA.conf")))
	{
		systemPause ();
		return 1;
	}

	//
	// Extract lexemes from source
	//
	if (!lex->loadFile (sourceFileName.c_str ()))
	{
		systemPause ();
		return 2;
	}

	if (saveLexemeFile)
		lex->saveLexemeFile (_TEXT ("lexeme.txt"));

	systemPause ();

	//
	// Create and initialize parser
	//
	std::unique_ptr<MpParser> psr (new MpParser (lex.get ()));
	psr->parse ();
	systemPause ();

	//
	// Create and initialize POLIR converter and interpreter
	//
	std::unique_ptr<MpPolir> plr (new MpPolir (lex.get (), psr.get ()));
	plr->convertProgram ();

	if (savePolirFile)
		plr->saveToFile (_TEXT ("MpPolir.txt"));

	//
	// Execure converted program
	//
	plr->executeProgram ();
	
	systemPause ();
	return 0;
}