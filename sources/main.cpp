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

#include <Poco/Util/Application.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/PatternFormatter.h>
#include <Poco/Util/AbstractConfiguration.h>

using namespace MiniPascal;

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;

/**
  * @brief The MiniPascal interpreter application
  *
  * This class do following things: 
  * - parse command line arguments
  * - load lexer settings from configuration file
  * - split the source text into lexeme table (lexer)
  * - check the MiniPascal language syntax corectness (parser)
  * - check source semantics correctness (semler)
  * - convert source to POLIR
  * - execure POLIR
  *
  * @remark Try MiniPascalInterpreter --help (on Unix platforms) or MiniPascalInterpreter /help (elsewhere)
  * for more information
  */
class MpInterpreterApp : public Application
{
	bool m_help_requested = false;
	bool m_verbose_mode = false;
	std::string m_lexeme_file;
	std::string m_polir_file;

public:
	MpInterpreterApp () : m_help_requested (false)
	{
	}

protected:
	virtual void initialize (Application& _self)
	{
		//
		// Load default configuration files, if present
		//
		auto ini_loaded_cnt = loadConfiguration ();
		Application::initialize (_self);

		//
		// TODO: Add your own initialization code here
		//
	}

	virtual void uninitialize ()
	{
		//
		// TODO: add your own uninitialization code here
		//
		Application::uninitialize ();
	}

	virtual void reinitialize (Application& _self)
	{
		Application::reinitialize (_self);

		//
		// TODO: add your own reinitialization code here
		//
	}

	virtual void defineOptions (OptionSet& _options)
	{
		Application::defineOptions (_options);

		_options.addOption (
			Option ("help", "h", "display help information on command line arguments")
			.required (false)
			.repeatable (false)
			.callback (OptionCallback<MpInterpreterApp> (this, &MpInterpreterApp::handleHelp)));

		_options.addOption (
			Option ("verbose", "v", "allow extended output for lexer, parser/semler and POLIR converter and interpreter")
			.required (false)
			.repeatable (false)
			.callback (OptionCallback<MpInterpreterApp> (this, &MpInterpreterApp::handleVerbose)));

		_options.addOption (
			Option ("lexeme-file", "l", "save lexeme data to the specified file")
			.required (false)
			.repeatable (false)
			.argument ("file")
			.callback (OptionCallback<MpInterpreterApp> (this, &MpInterpreterApp::handleLexemeFile)));

		_options.addOption (
			Option ("polir-file", "p", "save POLIR data to the specified file")
			.required (false)
			.repeatable (false)
			.argument ("file")
			.callback (OptionCallback<MpInterpreterApp> (this, &MpInterpreterApp::handlePolirFile)));
	}

	void handleVerbose (const std::string& _name, const std::string& _value)
	{
		//
		// Save the verbose mode state
		//
		m_verbose_mode = true;
	}

	void handleLexemeFile (const std::string& _name, const std::string& _value)
	{
		//
		// Save the lexeme file name
		//
		m_lexeme_file = _value;
	}

	void handlePolirFile (const std::string& _name, const std::string& _value)
	{
		//
		// Save the POLIR file name
		//
		m_polir_file = _value;
	}

	void handleHelp (const std::string& _name, const std::string& _value)
	{
		m_help_requested = true;

		displayHelp ();
		stopOptionsProcessing ();
	}

	void handleConfig (const std::string& _name, const std::string& _value)
	{
		loadConfiguration (_value);
	}

	void displayHelp ()
	{
		//
		// TODO: write a complete and clear help text here
		//
		HelpFormatter helpFormatter (options ());
		helpFormatter.setCommand (commandName ());
		helpFormatter.setUsage ("OPTIONS");
		helpFormatter.setHeader ("Interpreter of very limited subset of the famous Pascal language");
		helpFormatter.format (std::cout);
	}

	virtual int main (const std::vector<std::string>& _args)
	{
		if (!m_help_requested)
		{
			//
			// Setup the logstream object to use pretty colored output channel, under both Unix and Windows
			//
#ifdef _WIN32
			Poco::AutoPtr<Poco::WindowsColorConsoleChannel> color_channel (new Poco::WindowsColorConsoleChannel ());
#else
			Poco::AutoPtr<Poco::ColorConsoleChannel> color_channel (new Poco::ColorConsoleChannel ());
#endif
			Poco::AutoPtr<Poco::PatternFormatter> pattern_formatter (new Poco::PatternFormatter);
			pattern_formatter->setProperty ("pattern", "[%H:%M:%S] [%p] %t");
			Poco::AutoPtr<Poco::FormattingChannel> formatting_channel (new Poco::FormattingChannel (pattern_formatter, color_channel));
			logger ().setChannel (formatting_channel);
			//			logger ().setChannel (color_channel);

			//
			// Create stream interface object for STL stream-like output
			//
			Poco::LogStream ls (logger ());

			//
			// Enable output of debug info if appropriate parameter was specified
			//
			if (m_verbose_mode)
			{
				logger ().setLevel (Poco::Message::PRIO_DEBUG);
				ls.debug () << "Verbose mode was enabled" << std::endl;

				//
				// Show encoding name globally used by Poco library
				//
				Poco::TextEncoding& te = Poco::TextEncoding::global ();
				ls.debug () << "The global interpreter encoding: " << te.canonicalName () << std::endl;
			}

			//
			// Save the MiniPascal sources names from positional arguments
			//
			// NOTE:  Positional arguments are unnamed arguments on the command line, coming at the end after all other options
			//
			std::vector<std::string> mp_sources = _args;

			//
			// Interpreter must have at least one source file to parse
			//
			if (mp_sources.empty ())
			{
				ls.error () << "ERROR: Pascal source code file was not specified" << std::endl;
				UnicodeConsole::instance ().pause ();
				exit (0);
			}

			//
			// Let's start: create and initialize the lexer object
			//
			std::unique_ptr<MpLexer> lex (new MpLexer (ls));
			if (!lex->loadConfig (config ()))
			{
				UnicodeConsole::instance ().pause ();
				return 1;
			}

			for (auto src : mp_sources)
			{
				//
				// Set the appropriate console window title
				//
				UnicodeConsole::instance ().setTitle (Poco::Path (src).getFileName () + " - MiniPascal interpreter");

				//
				// Extract lexemes from source
				//
				if (!lex->loadFile (src))
				{
//					UnicodeConsole::instance ().pause ();
					break;
				}

				//
				// Save the lexeme list into the file if required
				//
				if (!m_lexeme_file.empty ())
					lex->saveLexemeFile (m_lexeme_file);

				UnicodeConsole::instance ().pause ();

				//
				// Create and initialize parser
				//
				std::unique_ptr<MpParser> psr (new MpParser (lex.get (), ls));
				psr->parse ();
				UnicodeConsole::instance ().pause ();

				//
				// Create and initialize POLIR converter and interpreter
				//
				std::unique_ptr<MpPolir> plr (new MpPolir (lex.get (), psr.get (), ls));
				plr->convertProgram ();

				//
				// Save the POLIR tokens into the file if required
				//
				if (!m_polir_file.empty ())
					plr->saveToFile (m_polir_file);

				//
				// Execure converted program
				//
				plr->executeProgram ();
			}
		}

		UnicodeConsole::instance ().pause ();
		return Application::EXIT_OK;
	}
};

POCO_APP_MAIN (MpInterpreterApp)

