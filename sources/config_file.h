/**
 * @file
 * @brief Interface of text configuration file reader
 * @note Ini-like format is used
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "types.h"

#ifndef __MINIPASCAL_CONFIG_H
#define __MINIPASCAL_CONFIG_H

namespace MiniPascal
{
	class MpConfigFile
	{
		MpInputFileStream file;

	public:
		/**
		 * @brief Open configuration file in the read-only mode
		 */
		bool openFile (const MpChar* name);

		/**
		 * @brief Close previosly opened configuration file
		 */
		void closeFile ();

		/**
		 * @name Read the entire section of settings
		 */
		/** @{*/
		bool readSection (const MpString& section, MpStringList & dst);
		bool readSection (const MpString& section, MpString * dst, long count);
		/** @}*/
	};
}

#endif // __MINIPASCAL_CONFIG_H