/**
 * @file
 * @brief The simple implementation of text configuration file reader
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */   

#include "config_file.h"

using namespace std;
using namespace MiniPascal;

bool
MpConfigFile::openFile (const MpChar* name)
{
	file.open (name);

	return file.is_open ();
}

void
MpConfigFile::closeFile ()
{
	file.close ();
}

bool
MpConfigFile::readSection (const MpString& section, MpStringList& dst)
{
	MpString line;
	MpString section_lower = section;
	toLower (section_lower);

	//
	// Set position pointer to file begin
	//
	file.clear ();
    file.seekg (0L, ios::beg);

	if (!file)
	{
		MpCout << _TEXT ("MpConfigFile::readSection() -- invalid file") << endl;
		return false;
	}

	//
	// Process all file line-by-line sequentially
	//
	while (!file.eof ())
	{
		//
		// Read the next line in the file
		//
		getline (file, line);

		//
		// Skip empty ones
		//
		if (line.empty ())
			continue;

		toLower (line);
		if (line == section_lower)
		{
			while (!file.eof ())
			{
				getline (file, line);
				if (line.empty ())
					continue;

				if ((line [0] == MP_SECTION_BEGIN) /* || (line [0] == '\n')*/)
					return true;

				toLower(line);
				dst.push_back (line);
			}
		}
	}

	return true;
}
