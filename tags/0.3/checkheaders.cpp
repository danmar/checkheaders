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
#include "checkheaders.h"
#include "tokenize.h"
#include "commoncheck.h"
#include <algorithm>
#include <set>
#include <list>
#include <sstream>
#include <string>
#include <cstring>
#include <iostream>
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// HEADERS - No implementation in a header
//---------------------------------------------------------------------------

void WarningHeaderWithImplementation(const Tokenizer &tokenizer, bool XmlOutput, std::ostream &errout)
{
    for (const Token *tok = tokenizer.tokens; tok; tok = tok->next)
    {
        // Only interested in included file
        if (tok->FileIndex == 0)
            continue;

        if (Match(tok, ") {"))
        {
            std::ostringstream ostr;
            ostr << "Found implementation in header";
            ReportErr(tokenizer, XmlOutput, tok, "HeaderWithImplementation", ostr.str(), errout);
        }
    }
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// HEADERS - Unneeded include
//---------------------------------------------------------------------------

static std::string matchSymbols(const std::set<std::string> &needed, const std::set<std::string> & classes, const std::set<std::string> & names)
{
    for (std::set<std::string>::const_iterator sym = needed.begin(); sym != needed.end(); ++sym)
    {
        if (classes.find(*sym) != classes.end() ||
            names.find(*sym) != names.end())
        {
            return *sym;
        }
    }
    return "";
}

class IncludeInfo
{
public:
    IncludeInfo(const Token *t, unsigned int i)
    {
        tok = t;
        hfile = i;
    }

    IncludeInfo(const IncludeInfo &info)
    {
        tok = info.tok;
        hfile = info.hfile;
    }
    
    const Token *tok;
    unsigned int hfile;
};

static void getincludes(const std::vector< std::list<IncludeInfo> > &includes, const unsigned int hfile, std::set<unsigned int> &result, bool &notfound)
{
    if (hfile < includes.size())
    {
        for (std::list<IncludeInfo>::const_iterator it = includes[hfile].begin(); it != includes[hfile].end(); ++it)
        {
            if (it->hfile >= includes.size())
                notfound = true;
            else if (result.find(it->hfile) == result.end())
            {
                result.insert(it->hfile);
                getincludes(includes, it->hfile, result, notfound);
            }
        }
    }
}

void WarningIncludeHeader(const Tokenizer &tokenizer, bool Progress, bool XmlOutput, std::ostream &errout)
{
    // A header is needed if:
    // * It contains some needed class declaration
    // * It contains some needed function declaration
    // * It contains some needed constant value
    // * It contains some needed variable
    // * It contains some needed enum

    // class names..
    std::vector< std::set<std::string> > classes(tokenizer.ShortFileNames.size(), std::set<std::string>());

    // symbol names..
    std::vector< std::set<std::string> > names(tokenizer.ShortFileNames.size(), std::set<std::string>());

    // Extract symbols from the files..
    {
        int indentlevel = 0;
        for (const Token *tok = tokenizer.tokens; tok; tok = tok->next)
        {
            // Don't extract symbols in the main source file
            if (tok->FileIndex == 0)
                continue;
            
            if (tok->str[0] == '{')
                indentlevel++;

            else if (tok->str[0] == '}')
                indentlevel--;

            if (indentlevel != 0)
                continue;

            // Class or namespace declaration..
            // --------------------------------------
            if (Match(tok,"class %var% {") || Match(tok,"class %var% :") || Match(tok,"struct %var% {"))
                classes[tok->FileIndex].insert(getstr(tok, 1));

            else if (Match(tok, "struct %var% ;") || Match(tok, "class %var% ;"))
                continue;

            // Variable declaration..
            // --------------------------------------
            else if (Match(tok, "%type% %var% ;") || Match(tok, "%type% %var% ["))
                names[tok->FileIndex].insert(getstr(tok, 1));

            else if (Match(tok, "%type% * %var% ;") || Match(tok, "%type% * %var% ["))
                names[tok->FileIndex].insert(getstr(tok, 2));

            // enum..
            // --------------------------------------
            else if (strcmp(tok->str, "enum") == 0)
            {
                tok = tok->next;
                while (tok->next && tok->str[0]!=';')
                {
                    if ( IsName(tok->str) )
                        names[tok->FileIndex].insert(tok->str);
                    tok = tok->next;
                }
            }
                
            // function..  
            // --------------------------------------
            else if (Match(tok,"%type% %var% (") ||
                     Match(tok,"%type% * %var% ("))
            {
                tok = tok->next;
                if (tok->str[0] == '*')
                    tok = tok->next;
                names[tok->FileIndex].insert(tok->str);
                unsigned int parlevel = 0;
                while (tok->next)
                {
                    if (tok->str[0] == '(')
                        ++parlevel;
                    else if (tok->str[0] == ')')
                    {
                        --parlevel;
                        if (parlevel == 0)
                            break;
                    }
                    tok = tok->next;
                }
            }

            // typedef..
            // --------------------------------------
            else if (strcmp(tok->str,"typedef")==0)
            {
                if (strcmp(getstr(tok,1),"enum")==0)
                    continue;
                int parlevel = 0;
                while (tok->next)
                {
                    if ( strchr("({", tok->str[0]) )
                        parlevel++;

                    else if ( strchr(")}", tok->str[0]) )
                        parlevel--;

                    else if (parlevel == 0)
                    {
                        if ( tok->str[0] == ';' )
                            break;

                        if ( Match(tok, "%var% ;") )
                            names[tok->FileIndex].insert(tok->str);
                    }
    
                    tok = tok->next;
                }
            }

            // #define..
            // --------------------------------------
            else if (Match(tok, "#define %var%"))
                names[tok->FileIndex].insert(tok->next->str);
        }
    }


    // System headers are checked differently..
    std::vector<unsigned int> SystemHeaders(tokenizer.ShortFileNames.size(), 0);
    for (const Token *tok = tokenizer.tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str, "#include<>") == 0 ||
            (SystemHeaders[tok->FileIndex] && strcmp(tok->str, "#include") == 0))
        {
            // Get index of included file:
            const char *includefile = tok->next->str;
            for (unsigned int hfile = 0; hfile < tokenizer.ShortFileNames.size(); ++hfile)
            {
                if ( SameFileName( tokenizer.ShortFileNames[hfile].c_str(), includefile ) )
                {
                    SystemHeaders[hfile] = 1;
                    break;
                }
            }
        }
    }

    // Get all needed symbols..
    std::vector< std::set<std::string> > needed(tokenizer.ShortFileNames.size(), std::set<std::string>() );
    std::vector< std::set<std::string> > needDeclaration(tokenizer.ShortFileNames.size(), std::set<std::string>() );
    {
        // Which files contain implementation?
        std::vector<unsigned int> HasImplementation(tokenizer.ShortFileNames.size(), 0);
    
        int indentlevel = 0;
        for (const Token *tok1 = tokenizer.tokens; tok1; tok1 = tok1->next)
        {
            if (strncmp(tok1->str, "#include", 8) == 0)
            {
                tok1 = tok1->next;
                continue;
            }

            // implementation begins..
            if (indentlevel == 0 && Match(tok1, ") {"))
            {
                // Go to the "{"
                while (tok1->str[0] != '{')
                    tok1 = tok1->next;
                indentlevel = 1;
                HasImplementation[tok1->FileIndex] = 1;
			}
            else if (indentlevel >= 1)
            {
                if (tok1->str[0] == '{')
                    ++indentlevel;
                else if (tok1->str[0] == '}')
                    --indentlevel;
            }

            if ( Match(tok1, ": %var% {") || Match(tok1, ": %type% %var% {") )
            {
                const std::string classname(getstr(tok1, (strcmp(getstr(tok1,2),"{")) ? 2 : 1));
                needed[tok1->FileIndex].insert(classname);
            }

            if (indentlevel == 0 && Match(tok1, "%type% * %var%"))
            {
                if (Match(gettok(tok1,3), "[,;()[]"))
                {
                    needDeclaration[tok1->FileIndex].insert(tok1->str);
                    tok1 = gettok(tok1, 2);
                    continue;
                }
            }

            if (Match(tok1, "struct") || Match(tok1, "class"))
            {
                continue;
            }

            if ( IsName(tok1->str) && !Match(tok1->next, "{") )
                needed[tok1->FileIndex].insert(tok1->str);
        }

        // Move needDeclaration symbols to needed for all files that has
        // implementation..
        for (unsigned int i = 0; i < HasImplementation.size(); ++i)
        {
            if (HasImplementation[i])
            {
                needed[i].insert(needDeclaration[i].begin(), needDeclaration[i].end());
            }
        }
    }

    // Remove keywords..
    for (unsigned int i = 0; i < tokenizer.ShortFileNames.size(); ++i)
    {
        const char *keywords[] = { "defined", // preprocessor
                                   "void", "bool", "char", "short", "int", "long", "float", "double",
                                   "false", "true",
                                   "std",
                                   NULL
                                 };
                                 
        for (unsigned int k = 0; keywords[k]; ++k)
        {
            needed[i].erase(keywords[k]);
            needDeclaration[i].erase(keywords[k]);
        }
    }

    // Extract all includes..
    std::vector< std::list<IncludeInfo> > includes(tokenizer.ShortFileNames.size(), std::list< IncludeInfo >());
    for (const Token *tok = tokenizer.tokens; tok; tok = tok->next)
    {
        if (strncmp(tok->str, "#include", 8) == 0)
        {
            // Get index of included file:
            unsigned int hfile;
            const char *includefile = tok->next->str;
            for (hfile = 0; hfile < tokenizer.ShortFileNames.size(); ++hfile)
            {
                if ( SameFileName( tokenizer.ShortFileNames[hfile].c_str(), includefile ) )
                    break;
            }
            includes[tok->FileIndex].push_back(IncludeInfo(tok, hfile));
        }
    }

    // Check if there are redundant includes..
    for (unsigned int fileIndex = 0; fileIndex < tokenizer.ShortFileNames.size(); ++fileIndex)
    {
        // Is the current file a system header? If so don't check it.
        if (SystemHeaders[fileIndex])
            continue;

        for (std::list<IncludeInfo>::const_iterator include = includes[fileIndex].begin(); include != includes[fileIndex].end(); ++include)
        {
            // include not found
            if (include->hfile >= tokenizer.ShortFileNames.size())
                continue;

            if (Progress)
            {
                std::cout << "progress: file " << tokenizer.ShortFileNames[fileIndex] << " checking include " << tokenizer.ShortFileNames[include->hfile] << std::endl;
            }

            // Get all includes
            std::set<unsigned int> AllIncludes;
            bool notfound = false;
            AllIncludes.insert(include->hfile);
            if (SystemHeaders[include->hfile])
                getincludes(includes, include->hfile, AllIncludes, notfound);

            // match symbols: needed
            bool Needed(false);
            for (std::set<unsigned int>::const_iterator it = AllIncludes.begin(); it != AllIncludes.end(); ++it)
            {
                const std::string sym = matchSymbols(needed[fileIndex], classes[*it], names[*it]);
                if (!sym.empty())
                {
                    if (Progress)
                        std::cout << "progress: needed symbol '" << sym << "'" << std::endl;
                    Needed = true;
                    break;
                }
            }
            if (!Needed)
            {
                if (!notfound)
                {
                    bool NeedDeclaration(false);
                    for (std::set<unsigned int>::const_iterator it = AllIncludes.begin(); it != AllIncludes.end(); ++it)
                    {
                        std::set<std::string> empty;
                        const std::string sym = matchSymbols(needDeclaration[fileIndex], classes[*it], empty);
                        if (!sym.empty())
                        {
                            NeedDeclaration = true;
                            break;
                        }
                    }

                    std::ostringstream errmsg;
                    errmsg << "The included header '" << include->tok->next->str << "' is not needed";
                    if (NeedDeclaration)
                        errmsg << " (but forward declaration is needed)";

                    ReportErr(tokenizer, XmlOutput, include->tok, "HeaderNotNeeded", errmsg.str(), errout);
                }
                else if (Progress)
                    std::cout << "progress: bail out (header not found)" << std::endl;
            }
        }
    }
}
//---------------------------------------------------------------------------



