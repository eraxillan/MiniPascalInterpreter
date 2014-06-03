/**
  * @file
  * @brief The interface of crossplatform Unicode aware console (UTF-8 encoding)
  * @author Alexander Kamyshnikov <axill777@gmail.com>
  */

#include <cstdlib>
#include <cstdio>

#include <string>

#include <Poco/SingletonHolder.h>
#include <Poco/UnicodeConverter.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

namespace MiniPascal
{
	class UnicodeConsole
	{
		bool m_pause_enabled = false;

		//UnicodeConsole ();

	public:
		//
		// FIXME: Poco SingletonHolder create object using it's constructor;
		// it break Singleton pattern "purity", i.e. client can create it's own instance of class
		//
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

			//
			// TODO: set the Lucida Console font 'cause it support Unicode chars
			//
#endif
		}

		static UnicodeConsole& instance ()
		{
			static Poco::SingletonHolder<UnicodeConsole> sh;
			return (*sh.get ());
		}


		/**
		  * @brief Write the specified UTF-8 string to the @a std::cout
		  * @note Under Windows function do the UTF-8 --> UTF-16 conversion
		  */
		void
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

		// TODO: formatable version of writeLine

		/**
		  * @brief Read a string from the @a std::cin and return it as UTF-8 encoded one
		  * @note Under Windows function do the UTF-16 --> UTF-8 conversion
		  */
		std::string
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
		  * @param[in] _text The new console title (UTF-8 encoded)
		  */
		void
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

		void
		enablePause (bool _enabled = true)
		{
			m_pause_enabled = _enabled;
		}

		/**
		  * @brief Suspends processing of a console program
		  * and displays a message prompting the user to press any key to continue
		  * @note Use @a enablePause (false) to turn this command to the stub (useful for execution in batch files)
		  * @note Use this crossplatform solution instead of Windows-only @a system ("pause")
		  */
		void
		pause ()
		{
			if (m_pause_enabled)
			{
				std::wcout << L"Press any key to continue..." << std::endl;
				std::wcin.clear ();
				std::wcin.sync ();
				std::wcin.get ();
			}
		}
	};
}
