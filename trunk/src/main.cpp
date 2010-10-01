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
#include <set>


static bool Debug;      /// --debug
static OutputFormat outputFormat;

static void CheckFile(const char FileName[], const std::vector<std::string> &includePaths, const std::set<std::string> &skipIncludes);

//---------------------------------------------------------------------------
// Main function of checkheaders
//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    std::vector<std::string> filenames;
    std::vector<std::string> includePaths;
    std::set<std::string> skipIncludes;

    outputFormat = OUTPUT_FORMAT_NORMAL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug") == 0)
        {
            Debug = true;
        }

        else if (strcmp(argv[i], "--skip") == 0 && (i + 1) < argc)
        {
            ++i;
            skipIncludes.insert(argv[i]);
        }

        else if (strcmp(argv[i], "--xml") == 0)
        {
            outputFormat = OUTPUT_FORMAT_XML;
        }

        else if (strcmp(argv[i], "--vs") == 0)
        {
            outputFormat = OUTPUT_FORMAT_VS;
        }

        else if (strchr("-/", *argv[i]) && *(argv[i]+1) == 'I')
        {
            // -I <dir
            if (*(argv[i]+2) == 0)
            {
                if ((i + 1) >= argc)
                {
                    std::cerr << "checkheaders: failed to parse '" << argv[i] << "'" << std::endl;
                    return 1;
                }
                ++i;
                includePaths.push_back(argv[i]);
            }

            // -I<dir>
            else
            {
                includePaths.push_back(argv[i] + 2);
            }
        }

        else if (strncmp(argv[i], "-", 1) == 0)
        {
            std::cerr << "checkheaders: unrecognized option: '" << argv[i] << "'" << std::endl;
            return 0;
        }

        else
        {
            unsigned int sz = filenames.size();
            FileLister::recursiveAddFiles(filenames, argv[i], true);
            if (sz == filenames.size())
            {
                std::cerr << "checkheaders: file/path not found: '" << argv[i] << "'" << std::endl;
                return 0;
            }
        }
    }

    if (filenames.empty())
    {
        std::cout << "check headers in C/C++ code to detect unnecessary includes.\n"
                  << "\n"
                  << "Syntax:\n"
                  << "    checkheaders [-I <path>] [--skip <file>] [--xml] <path or file>\n"
                  << "\n"
                  << "Options:\n"
                  << "    -I <path>      Specify include path. It is only needed if\n"
                  << "                   you see 'Header not found' messages.\n"
                  << "    --skip <file>  Skip header. Matching #include directives in\n"
                  << "                   the source code will be skipped.\n"
                  << "    --vs           Output report in visual studio format\n"
                  << "    --xml          Output report in xml format\n"
                  << "\n"
                  << "Example usage:\n"
                  << "    # check all files recursively under myproject\n"
                  << "    checkheaders myproject/\n"
                  << "    # Search for headers in the \"inc1\" folder\n"
                  << "    checkheaders -I inc1 myproject/\n"
                  << "    # Save error messages in a file\n"
                  << "    checkheaders myproject/ 2> report.txt\n";
        return 0;
    }

    std::sort(filenames.begin(), filenames.end());

    if (outputFormat == OUTPUT_FORMAT_XML)
    {
        std::cerr << "<?xml version=\"1.0\"?>\n"
                  << "<results>\n";
    }

    for (unsigned int c = 0; c < filenames.size(); c++)
    {
        CheckFile(filenames[c].c_str(), includePaths, skipIncludes);
    }

    if (outputFormat == OUTPUT_FORMAT_XML)
        std::cerr << "</results>\n";

    return 0;
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

static void CheckFile(const char FileName[], const std::vector<std::string> &includePaths, const std::set<std::string> &skipIncludes)
{
    std::cout << "Checking " << FileName << "...\n";

    // Tokenize the file
    Tokenizer tokenizer;
    tokenizer.tokenize(FileName, includePaths, skipIncludes, outputFormat, std::cerr);

    // debug output..
    if (Debug)
    {
        std::cout << "debug:";
        for (const Token *tok = tokenizer.tokens; tok; tok = tok->next)
            std::cout << " " << tok->str;
        std::cout << "\n";
    }

    // Including header which is not needed
    WarningIncludeHeader(tokenizer, true, outputFormat, std::cerr);
}
//---------------------------------------------------------------------------




