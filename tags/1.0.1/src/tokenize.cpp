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
#include "tokenize.h"
#include "commoncheck.h"    // <- IsName
//---------------------------------------------------------------------------

#include <iterator>   // back_inserter

#include <locale>
#include <fstream>

#include <string>
#include <cstring>
#include <cctype>

#include <string.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

// Helper functions..


static void combine_2tokens(Token *tok, const char str1[], const char str2[]);

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// addtoken
// add a token. Used by 'Tokenizer'
//---------------------------------------------------------------------------

void Tokenizer::addtoken(const char str[], const unsigned int lineno, const unsigned int fileno)
{
    if (str[0] == 0)
        return;

    // don't add "const"
    if (strcmp(str, "const") == 0)
        return;

    // Replace hexadecimal value with decimal
    std::ostringstream str2;
    if (strncmp(str,"0x",2)==0)
    {
        str2 << strtoul(str+2, NULL, 16);
    }
    else
    {
        str2 << str;
    }

    Token *newtoken  = new Token;
    std::memset(newtoken, 0, sizeof(Token));
    newtoken->str    = strdup(str2.str().c_str());
    newtoken->linenr = lineno;
    newtoken->FileIndex = fileno;
    if (tokens_back)
    {
        tokens_back->next = newtoken;
        tokens_back = newtoken;
    }
    else
    {
        tokens = tokens_back = newtoken;
    }
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// combine_2tokens
// Combine two tokens that belong to each other. Ex: "<" and "=" may become "<="
//---------------------------------------------------------------------------

static void combine_2tokens(Token *tok, const char str1[], const char str2[])
{
    if (!(tok && tok->next))
        return;
    if (strcmp(tok->str,str1) || strcmp(tok->next->str,str2))
        return;

    free(tok->str);
    std::string newstr(std::string(str1) + std::string(str2));
    tok->str = strdup(newstr.c_str());

    // Delete next token
    Token *next = tok->next;
    tok->next = next->next;
    free(next->str);
    delete next;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Tokenizer
//---------------------------------------------------------------------------

Tokenizer::Tokenizer()
{
    tokens = NULL;
    tokens_back = NULL;
}

Tokenizer::~Tokenizer()
{
    while (tokens)
    {
        Token *next = tokens->next;
        free(tokens->str);
        delete tokens;
        tokens = next;
    }
}

bool Tokenizer::tokenize(const char FileName[], const std::vector<std::string> &includePaths, const std::set<std::string> &skipIncludes, const OutputFormat outputFormat, std::ostream &errout)
{
    // Skip stdafx.h..
    if (SameFileName(FileName, "stdafx.h"))
        return true;

    // Has this file been tokenized already?
    for (unsigned int i = 0; i < ShortFileNames.size(); i++)
    {
        if (SameFileName(ShortFileNames[i].c_str(), FileName))
            return true;
    }

    std::string filename(FileName);

    // Open file..
    std::ifstream fin(FileName);
    if (!fin.is_open())
    {
        for (unsigned int i = 0; i < includePaths.size(); ++i)
        {
            filename = includePaths[i];

            // Append '/' if the last char is neither '/' nor '\'
            char lastChar = '/';
            if (!filename.empty())
                lastChar = filename[filename.size() - 1];
            if (lastChar != '\\' && lastChar != '/')
                filename += '/';

            // Append FileName
            filename += FileName;

            // Try to open file
            fin.clear();
            fin.open(filename.c_str());
            if (fin.is_open())
                break;
        }

        if (!fin.is_open())
            return false;
    }

    // The "Files" vector remembers what files have been tokenized..
    ShortFileNames.push_back(FileName);
    FullFileNames.push_back(filename);

    // Tokenize the file..
    tokenizeCode(fin, FullFileNames.size() - 1, includePaths, skipIncludes, outputFormat, errout);

    return true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Tokenize - tokenizes input stream
//---------------------------------------------------------------------------

void Tokenizer::tokenizeCode(std::istream &code, const unsigned int FileIndex, const std::vector<std::string> &includePaths, const std::set<std::string> &skipIncludes, const OutputFormat outputFormat, std::ostream &errout)
{
    // Tokenize the file.
    unsigned int lineno = 1;
    char CurrentToken[1000] = {0};
    char *pToken = CurrentToken;
    for (char ch = (char)code.get(); !code.eof(); ch = (char)code.get())
    {
        // Todo
        if (ch < 0)
            continue;

        // Preprocessor stuff?
        if (ch == '#' && !CurrentToken[0])
        {
            std::string line;
            do
            {
                line += ch;
                ch = (char)code.get();
            }
            while (!code.eof() && std::isalpha(ch));

            if (line.compare(0, 8, "#include")==0)
            {
                getline(code, line);
                if (line.find("//") != std::string::npos)
                    line.erase(line.find("//"));

                if (line.find_first_of("<\"") != std::string::npos)
                {
                    const bool SystemHeader(line.find("<") != std::string::npos);

                    // Extract the filename
                    line.erase(0, line.find_first_of("<\"")+1);
                    line.erase(line.find_first_of(">\""));

                    if (skipIncludes.find(line) == skipIncludes.end())
                    {
                        // Add path for current file to the include paths..
                        std::vector<std::string> incpaths;
                        if (FullFileNames[FileIndex].find_first_of("\\/") != std::string::npos)
                        {
                            std::string path = FullFileNames[FileIndex];
                            path.erase(1 + path.find_last_of("\\/"));
                            incpaths.push_back(path);
                        }
                        std::copy(includePaths.begin(), includePaths.end(), std::back_inserter(incpaths));

                        addtoken(SystemHeader ? "#include<>" : "#include", lineno, FileIndex);
                        addtoken(line.c_str(), lineno, FileIndex);

                        const bool found(tokenize(line.c_str(), incpaths, skipIncludes, outputFormat, errout));
                        if (!found)
                        {
                            free(tokens_back->str);
                            tokens_back->str = strdup("not found");
                            const std::string errmsg("Header not found '" + line + "'. Use -I or --skip to fix this message.");
                            ReportErr(outputFormat, FullFileNames[FileIndex], lineno, "HeaderNotFound", errmsg, errout);
                        }
                    }
                }
                ++lineno;
            }

            else
            {
                addtoken(line.c_str(), lineno, FileIndex);
                pToken = CurrentToken;
                std::memset(CurrentToken, 0, sizeof(CurrentToken));
            }

            continue;
        }

        if (ch == '\n')
        {
            // Add current token..
            addtoken(CurrentToken, lineno++, FileIndex);
            std::memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }

        // Comments..
        if (ch == '/' && !code.eof())
        {
            bool newstatement = bool(strchr(";{}", CurrentToken[0]) != NULL);

            // Add current token..
            addtoken(CurrentToken, lineno, FileIndex);
            std::memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;

            // Read next character..
            ch = (char)code.get();

            // If '//'..
            if (ch == '/')
            {
                std::string comment;
                getline(code, comment);     // Parse in the whole comment

                // If the comment says something like "fred is deleted" then generate appropriate tokens for that
                comment = comment + " ";
                if (newstatement && comment.find(" deleted ")!=std::string::npos)
                {
                    // delete
                    addtoken("delete", lineno, FileIndex);

                    // fred
                    std::string::size_type pos1 = comment.find_first_not_of(" \t");
                    std::string::size_type pos2 = comment.find(" ", pos1);
                    std::string firstWord = comment.substr(pos1, pos2-pos1);
                    addtoken(firstWord.c_str(), lineno, FileIndex);

                    // ;
                    addtoken(";", lineno, FileIndex);
                }

                lineno++;
                continue;
            }

            // If '/*'..
            if (ch == '*')
            {
                char chPrev;
                ch = chPrev = 'A';
                while (!code.eof() && (chPrev!='*' || ch!='/'))
                {
                    chPrev = ch;
                    ch = (char)code.get();
                    if (ch == '\n')
                        lineno++;
                }
                continue;
            }

            // Not a comment.. add token..
            addtoken("/", lineno, FileIndex);
        }

        // char..
        if (ch == '\'')
        {
            // Add previous token
            addtoken(CurrentToken, lineno, FileIndex);
            std::memset(CurrentToken, 0, sizeof(CurrentToken));

            // Read this ..
            CurrentToken[0] = ch;
            CurrentToken[1] = (char)code.get();
            CurrentToken[2] = (char)code.get();
            if (CurrentToken[1] == '\\')
                CurrentToken[3] = (char)code.get();

            // Add token and start on next..
            addtoken(CurrentToken, lineno, FileIndex);
            std::memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;

            continue;
        }

        // String..
        if (ch == '\"')
        {
            addtoken(CurrentToken, lineno, FileIndex);
            std::memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            bool special = false;
            char c = ch;
            do
            {
                // Append token..
                if (pToken < &CurrentToken[sizeof(CurrentToken)-10])
                {
                    *pToken = c;
                    pToken++;
                }

                // Special sequence '\.'
                if (special)
                    special = false;
                else
                    special = (c == '\\');

                // Get next character
                c = (char)code.get();
            }
            while (!code.eof() && (special || c != '\"'));
            *pToken = '\"';
            addtoken(CurrentToken, lineno, FileIndex);
            std::memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }

        if (strchr("+-*/%&|^?!=<>[](){};:,.",ch))
        {
            addtoken(CurrentToken, lineno, FileIndex);
            std::memset(CurrentToken, 0, sizeof(CurrentToken));
            CurrentToken[0] = ch;
            addtoken(CurrentToken, lineno, FileIndex);
            std::memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }


        if (std::isspace(ch) || std::iscntrl(ch))
        {
            addtoken(CurrentToken, lineno, FileIndex);
            pToken = CurrentToken;
            std::memset(CurrentToken, 0, sizeof(CurrentToken));
            continue;
        }

        *pToken = ch;
        pToken++;
    }

    // Combine tokens..
    for (Token *tok = tokens; tok && tok->next; tok = tok->next)
    {
        combine_2tokens(tok, "<", "<");
        combine_2tokens(tok, ">", ">");

        combine_2tokens(tok, "&", "&");
        combine_2tokens(tok, "|", "|");

        combine_2tokens(tok, "+", "=");
        combine_2tokens(tok, "-", "=");
        combine_2tokens(tok, "*", "=");
        combine_2tokens(tok, "/", "=");
        combine_2tokens(tok, "&", "=");
        combine_2tokens(tok, "|", "=");

        combine_2tokens(tok, "=", "=");
        combine_2tokens(tok, "!", "=");
        combine_2tokens(tok, "<", "=");
        combine_2tokens(tok, ">", "=");

        combine_2tokens(tok, ":", ":");
        combine_2tokens(tok, "-", ">");

        combine_2tokens(tok, "private", ":");
        combine_2tokens(tok, "protected", ":");
        combine_2tokens(tok, "public", ":");
    }

    // Replace "->" with "."
    for (Token *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str, "->") == 0)
        {
            tok->str[0] = '.';
            tok->str[1] = 0;
        }
    }
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// Helper functions for handling the tokens list
//---------------------------------------------------------------------------

const Token *gettok(const Token *tok, int offset)
{
    while (tok && offset>0)
    {
        tok = tok->next;
        offset--;
    }
    return tok;
}
//---------------------------------------------------------------------------

const char *getstr(const Token *tok, int offset)
{
    tok = gettok(tok, offset);
    return tok ? tok->str : "";
}
//---------------------------------------------------------------------------


