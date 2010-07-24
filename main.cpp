/*
 * checkheaders - check headers in C/C++ code
 * Copyright (C) 2010 Daniel Marjamäki.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @brief check C/C++ headers
 *
 * This program will check either the file(s) specified or all source files in
 * all sub-directories
 *
 **/


#include "tokenize.h"   // <- Tokenizer
#include "commoncheck.h"

#include "checkheaders.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>


#ifdef __GNUC__
#include <glob.h>
#include <unistd.h>
#endif
#ifdef __BORLANDC__
#include <dir.h>
#endif
#ifdef _MSC_VER
#include <windows.h>
#endif

//---------------------------------------------------------------------------
bool Debug = false;
bool ShowAll = false;
bool CheckCodingStyle = false;
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[], unsigned int FileId);


static void AddFiles( std::vector<std::string> &filenames, const char path[], const char pattern[] )
{
    #ifdef __GNUC__
    glob_t glob_results;
    glob(pattern, 0, 0, &glob_results);
    for ( unsigned int i = 0; i < glob_results.gl_pathc; i++ )
    {
        std::ostringstream fname;
        fname << path << glob_results.gl_pathv[i];
        filenames.push_back( fname.str() );
    }
    globfree(&glob_results);
    #endif
    #ifdef __BORLANDC__
    struct ffblk f;
    for ( int done = findfirst(pattern, &f, 0); ! done; done = findnext(&f) )
    {
        std::ostringstream fname;
        fname << path << f.ff_name;
        filenames.push_back( fname.str() );
    }
    findclose(&f);
    #endif
    #ifdef _MSC_VER
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(pattern, &ffd);
	if (INVALID_HANDLE_VALUE != hFind) 
	{
		do
		{
	        std::ostringstream fname;
			fname << path << ffd.cFileName;
			filenames.push_back( fname.str() );
		}
		while (FindNextFile(hFind, &ffd) != 0);
	}
	#endif
}

static void RecursiveAddFiles( std::vector<std::string> &filenames, const char path[] )
{
    AddFiles( filenames, path, "*.cpp" );
    AddFiles( filenames, path, "*.cc" );
    AddFiles( filenames, path, "*.c" );

    #ifdef __GNUC__
    // gcc / cygwin..
    glob_t glob_results;
    glob("*", GLOB_MARK, 0, &glob_results);
    for ( unsigned int i = 0; i < glob_results.gl_pathc; i++ )
    {
        const char *dirname = glob_results.gl_pathv[i];
        if ( dirname[0] == '.' )
            continue;

        if ( strchr(dirname, '/') == 0 )
            continue;

        chdir( dirname );
        std::ostringstream curdir;
        curdir << path << dirname;
        RecursiveAddFiles( filenames, curdir.str().c_str() );
        chdir( ".." );
    }
    globfree(&glob_results);
    #endif
    #ifdef __BORLANDC__
    struct ffblk f ;
    for ( int done = findfirst("*", &f, FA_DIREC); ! done; done = findnext(&f) )
    {
        if ( f.ff_attrib != FA_DIREC || f.ff_name[0] == '.' )
            continue;
        chdir( f.ff_name );
        std::ostringstream curdir;
        curdir << path << f.ff_name << "/";
        RecursiveAddFiles( filenames, curdir.str().c_str() );
        chdir( ".." );
    }
    findclose(&f);
    #endif
    #ifdef _MSC_VER
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile("*", &ffd);
	if (INVALID_HANDLE_VALUE != hFind) 
	{
		do
		{
			if ( (ffd.cFileName[0]!='.') &&
				 (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				SetCurrentDirectory( ffd.cFileName );
				std::ostringstream curdir;
				curdir << path << ffd.cFileName << "/";
				RecursiveAddFiles( filenames, curdir.str().c_str() );
				SetCurrentDirectory( ".." );
			}
		}
		while (FindNextFile(hFind, &ffd) != 0);
	}
	#endif
}

//---------------------------------------------------------------------------
// Main function of checkheaders
//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    std::vector<std::string> filenames;

    for (int i = 1; i < argc; i++)
    {
        if (strchr(argv[i],'*'))
        {
            AddFiles( filenames, "", argv[i] );
        }

        else
        {
            filenames.push_back( argv[i] );
        }
    }

    if (filenames.empty())
    {
        std::cout << "check headers in C/C++ code\n"
                     "\n"
                     "Syntax:\n"
                     "    checkheaders [filename1] [filename2]\n";
        return 0;
    }

    std::sort( filenames.begin(), filenames.end() );

    for (unsigned int c = 0; c < filenames.size(); c++)
    {
        errout.str("");
        CppCheck(filenames[c].c_str(), c);
        std::cerr << errout.str();
    }

    return 0;
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[], unsigned int FileId)
{
    OnlyReportUniqueErrors = true;

    std::cout << "Checking " << FileName << "...\n";

    // Tokenize the file
    tokens = tokens_back = NULL;
    Files.clear();
    Tokenize(FileName);

    FillFunctionList(FileId);

    // Including header which is not needed
    WarningIncludeHeader();

    // Clean up tokens..
    DeallocateTokens();

    if ( errout.str().empty() )
        std::cout << "No errors found\n";
}
//---------------------------------------------------------------------------




