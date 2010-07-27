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


#include "filelister.h"

#include "tokenize.h"   // <- Tokenizer

#include "checkheaders.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>


static bool Debug;      /// --debug
static bool XmlOutput;  /// --xml

static void CppCheck(const char FileName[], unsigned int FileId);

//---------------------------------------------------------------------------
// Main function of checkheaders
//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    std::vector<std::string> filenames;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug") == 0)
        {
            Debug = true;
        }

        else if (strcmp(argv[i], "--xml") == 0)
        {
            XmlOutput = true;
        }

        else
        {
            FileLister::recursiveAddFiles(filenames, argv[i], true);            
        }
    }

    if (filenames.empty())
    {
        std::cout << "check headers in C/C++ code\n"
                     "\n"
                     "Syntax:\n"
                     "    checkheaders [--xml] [filename1] [filename2]\n";
        return 0;
    }

    std::sort( filenames.begin(), filenames.end() );

    if (XmlOutput)
    {
        std::cerr << "<?xml version=\"1.0\"?>\n"
                  << "<results>\n";
    }

    for (unsigned int c = 0; c < filenames.size(); c++)
    {
        CppCheck(filenames[c].c_str(), c);
    }

    if (XmlOutput)
        std::cerr << "</results>\n";

    return 0;
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[], unsigned int FileId)
{
    std::cout << "Checking " << FileName << "...\n";

    // Tokenize the file
    const Tokenizer tokenizer(FileName);

    // debug output..
    if (Debug)
    {
        std::cout << "debug:";
        for (const Token *tok = tokenizer.tokens; tok; tok = tok->next)
            std::cout << " " << tok->str;
        std::cout << "\n";
    }

    // Including header which is not needed
    WarningIncludeHeader(tokenizer, XmlOutput, std::cerr);
}
//---------------------------------------------------------------------------




