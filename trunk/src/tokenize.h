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

//---------------------------------------------------------------------------
#ifndef tokenizeH
#define tokenizeH
//---------------------------------------------------------------------------

#include <set>
#include <string>
#include <vector>

struct Token
{
    unsigned int FileIndex;
    char *str;
    unsigned int linenr;
    struct Token *next;
};

enum OutputFormat
{
    OUTPUT_FORMAT_NORMAL,
    OUTPUT_FORMAT_XML,
    OUTPUT_FORMAT_VS
};

class Tokenizer
{
private:
    struct Token * tokens_back;

    void tokenizeCode(std::istream &code, const unsigned int FileIndex, const std::vector<std::string> &includePaths, const std::set<std::string> &skipIncludes, const OutputFormat outputFormat, std::ostream &errout);

    void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno);

public:
    Tokenizer();
    ~Tokenizer();

    /**
     * tokenize a file
     * @param FileName file name
     * @param SystemHeader is this file included through <>
     * @param includePaths search paths for the file
     * @param skipIncludes skip #include that match
     * @param XmlOutput should errors be written in xml format?
     * @param errout error stream
     */
    bool tokenize(const char FileName[], const std::vector<std::string> &includePaths, const std::set<std::string> &skipIncludes, const OutputFormat outputFormat, std::ostream &errout);

    struct Token * tokens;
    std::vector<std::string> FullFileNames;
    std::vector<std::string> ShortFileNames;
};


// Helper functions for handling the tokens list..
const Token *gettok(const Token *tok, int index);
const char *getstr(const Token *tok, int index);


//---------------------------------------------------------------------------
#endif

