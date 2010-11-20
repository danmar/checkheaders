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
#include "commoncheck.h"
#include "tokenize.h"
#include <stdlib.h>     // free
#include <sstream>
#include <list>
#include <algorithm>
#include <cstring>

#ifdef __BORLANDC__
#include <ctype.h>
#endif
//---------------------------------------------------------------------------

extern bool XmlOutput;

//---------------------------------------------------------------------------

bool SameFileName(const char fname1[], const char fname2[])
{
#ifdef __linux__
    return bool(strcmp(fname1, fname2) == 0);
#endif
#ifdef __GNUC__
    return bool(strcasecmp(fname1, fname2) == 0);
#endif
#ifdef __BORLANDC__
    return bool(stricmp(fname1, fname2) == 0);
#endif
#ifdef _MSC_VER
    return bool(_stricmp(fname1, fname2) == 0);
#endif
}
//---------------------------------------------------------------------------

std::set<std::string> ErrorList;

void ReportErr(OutputFormat of, const std::string &file, const int line, const std::string &id, const std::string &errmsg, std::ostream &errout)
{
    std::ostringstream ostr;
    if (of == OUTPUT_FORMAT_XML)
    {
        ostr << "<error file=\"" << file << "\""
             << " line=\"" << line << "\""
             << " severity=\"style\""
             << " id=\"" << id << "\""
             << " msg=\"" << errmsg << "\">";
    }
    else
    {
        if (of == OUTPUT_FORMAT_NORMAL)
            ostr << "[" << file << ":" << line << "]";
        else if (of == OUTPUT_FORMAT_VS)
            ostr << file << "(" << line << ")";
        ostr << " (style): " << errmsg;
    }

    // Avoid duplicate error messages..
    if (ErrorList.find(ostr.str()) == ErrorList.end())
    {
        ErrorList.insert(ostr.str());
        errout << ostr.str() << std::endl;
    }
}


void ReportErr(const Tokenizer &tokenizer, OutputFormat of, const Token *tok, const std::string &id, const std::string &errmsg, std::ostream &errout)
{
    ReportErr(of, tokenizer.FullFileNames[tok->FileIndex], tok->linenr, id, errmsg, errout);
}
//---------------------------------------------------------------------------

bool IsName(const char str[])
{
    return bool(str[0]=='_' || isalpha(str[0]));
}
//---------------------------------------------------------------------------

bool IsNumber(const char str[])
{
    return bool(isdigit(str[0]) != 0);
}
//---------------------------------------------------------------------------

bool IsStandardType(const char str[])
{
    if (!str)
        return false;
    bool Ret = false;
    const char *type[] = {"bool","char","short","int","long","float","double",0};
    for (int i = 0; type[i]; i++)
        Ret |= (strcmp(str,type[i])==0);
    return Ret;
}
//---------------------------------------------------------------------------

bool Match(const Token *tok, const char pattern[])
{
    if (!tok)
        return false;

    const char *p = pattern;
    while (*p)
    {
        // Skip spaces in pattern..
        while (*p == ' ')
            p++;

        // Extract token from pattern..
        char str[50];
        char *s = str;
        while (*p && *p!=' ')
        {
            *s = *p;
            s++;
            p++;
        }
        *s = 0;

        // No token => Success!
        if (str[0] == 0)
            return true;

        // Any symbolname..
        if (strcmp(str,"%var%")==0 || strcmp(str,"%type%")==0)
        {
            if (!IsName(tok->str))
                return false;
        }

        else if (strcmp(str,"%num%")==0)
        {
            if (! IsNumber(tok->str))
                return false;
        }


        else if (strcmp(str,"%str%")==0)
        {
            if (tok->str[0] != '\"')
                return false;
        }

        // [.. => search for a one-character token..
        else if (str[0]=='[' && strchr(str, ']') && tok->str[1] == 0)
        {
            *strrchr(str, ']') = 0;
            if (strchr(str + 1, tok->str[0]) == 0)
                return false;
        }

        else if (strcmp(str, tok->str) != 0)
            return false;

        tok = tok->next;
        if (!tok)
            return false;
    }

    // The end of the pattern has been reached and nothing wrong has been found
    return true;
}
//---------------------------------------------------------------------------

void deleteTokens(Token *tok)
{
    while (tok)
    {
        Token *next = tok->next;
        free(tok->str);
        delete tok;
        tok = next;
    }
}
//---------------------------------------------------------------------------


