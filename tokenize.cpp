//---------------------------------------------------------------------------
#include "tokenize.h"
#include "commoncheck.h"    // <- IsName
//---------------------------------------------------------------------------

#include <locale>
#include <fstream>

#include <map>
#include <string>
#include <cstring>

#include <stdlib.h>     // <- strtoul
#include <stdio.h>

#ifdef __BORLANDC__
#include <ctype.h>
#include <mem.h>
#endif

#ifndef _MSC_VER
#define _strdup(str) strdup(str)
#endif

//---------------------------------------------------------------------------

// Helper functions..

static void Define(const char Name[], const char Value[]);

static void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno);

static void combine_2tokens(TOKEN *tok, const char str1[], const char str2[]);

static void DeleteNextToken(TOKEN *tok);

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

std::vector<std::string> Files;
struct TOKEN *tokens, *tokens_back;

//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Defined symbols.
// "#define abc 123" will create a defined symbol "abc" with the value 123
//---------------------------------------------------------------------------

struct DefineSymbol
{
    char *name;
    char *value;
    struct DefineSymbol *next;
};
static struct DefineSymbol * dsymlist;

static void Define(const char Name[], const char Value[])
{
    if (!(Name && Name[0]))
        return;

    if (!(Value && Value[0]))
        return;

    // Is 'Value' a decimal value..
    bool dec = true, hex = true;
    for (int i = 0; Value[i]; i++)
    {
        if ( ! isdigit(Value[i]) )
            dec = false;

        if ( ! isxdigit(Value[i]) && (!(i==1 && Value[i]=='x')))
            hex = false;
    }

    if (!dec && !hex)
        return;

    char *strValue = _strdup(Value);

    if (!dec && hex)
    {
		// Convert Value from hexadecimal to decimal
		unsigned long value;
		std::istringstream istr(Value+2);
		istr >> std::hex >> value;
		std::ostringstream ostr;
		ostr << value;
        free(strValue);
        strValue = _strdup(ostr.str().c_str());
    }

    DefineSymbol *NewSym = new DefineSymbol;
    memset(NewSym, 0, sizeof(DefineSymbol));
    NewSym->name = _strdup(Name);
    NewSym->value = strValue;
    NewSym->next = dsymlist;
    dsymlist = NewSym;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// addtoken
// add a token. Used by 'Tokenizer'
//---------------------------------------------------------------------------

static void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno)
{
    if (str[0] == 0)
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

    TOKEN *newtoken  = new TOKEN;
    memset(newtoken, 0, sizeof(TOKEN));
    newtoken->str    = _strdup(str2.str().c_str());
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

    // Check if str is defined..
    for (DefineSymbol *sym = dsymlist; sym; sym = sym->next)
    {
        if (strcmp(str,sym->name)==0)
        {
            free(newtoken->str);
            newtoken->str = _strdup(sym->value);
            break;
        }
    }
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// combine_2tokens
// Combine two tokens that belong to each other. Ex: "<" and "=" may become "<="
//---------------------------------------------------------------------------

static void combine_2tokens(TOKEN *tok, const char str1[], const char str2[])
{
    if (!(tok && tok->next))
        return;
    if (strcmp(tok->str,str1) || strcmp(tok->next->str,str2))
        return;

    free(tok->str);
	std::string newstr(std::string(str1) + std::string(str2));
	tok->str = _strdup( newstr.c_str() );

    DeleteNextToken(tok);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// DeleteNextToken. Unlink and delete next token.
//---------------------------------------------------------------------------

static void DeleteNextToken(TOKEN *tok)
{
    TOKEN *next = tok->next;
    tok->next = next->next;
    free(next->str);
    delete next;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Tokenize - tokenizes a given file.
//---------------------------------------------------------------------------

void Tokenize(const char FileName[])
{
    // Has this file been tokenized already?
    for (unsigned int i = 0; i < Files.size(); i++)
    {
        if ( SameFileName( Files[i].c_str(), FileName ) )
            return;
    }

    // Open file..
    std::ifstream fin(FileName);
    if (!fin.is_open())
        return;

    // The "Files" vector remembers what files have been tokenized..
    Files.push_back(FileName);

    // Tokenize the file..
    TokenizeCode( fin, Files.size() - 1 );
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Tokenize - tokenizes input stream
//---------------------------------------------------------------------------

void TokenizeCode(std::istream &code, const unsigned int FileIndex)
{
    // Tokenize the file.
    unsigned int lineno = 1;
    char CurrentToken[1000] = {0};
    char *pToken = CurrentToken;
    for (char ch = (char)code.get(); !code.eof(); ch = (char)code.get())
    {
		// Todo
		if ( ch < 0 )
			continue;

        // Preprocessor stuff?
        if (ch == '#' && !CurrentToken[0])
        {
            std::string line;
            getline(code,line);
            line = "#" + line;
            if (strncmp(line.c_str(),"#include",8)==0 &&
                line.find("\"") != std::string::npos)
            {
                // Extract the filename
                line.erase(0, line.find("\"")+1);
                line.erase(line.find("\""));

                // Relative path..
                if (Files.back().find_first_of("\\/") != std::string::npos)
                {
                    std::string path = Files.back();
                    path.erase( 1 + path.find_last_of("\\/") );
                    line = path + line;
                }

                addtoken("#include", lineno, FileIndex);
                addtoken(line.c_str(), lineno, FileIndex);

                Tokenize(line.c_str());
            }

            else if (strncmp(line.c_str(), "#define", 7) == 0)
            {
                char *strId = NULL;
                enum {Space1, Id, Space2, Value} State;
                State = Space1;
                for (unsigned int i = 8; i < line.length(); i++)
                {
                    if (State==Space1 || State==Space2)
                    {
                        if (isspace(line[i]))
                            continue;
                        State = (State==Space1) ? Id : Value;
                    }

                    else if (State==Id)
                    {
                        if ( isspace( line[i] ) )
                        {
                            strId = _strdup(CurrentToken);
                            memset(CurrentToken, 0, sizeof(CurrentToken));
                            pToken = CurrentToken;
                            State = Space2;
                            continue;
                        }
                        else if ( ! isalnum(line[i]) )
                        {
                            break;
                        }
                    }

                    *pToken = line[i];
                    pToken++;
                }

                if (State==Value)
                {
                    addtoken("def", lineno, FileIndex);
                    addtoken(strId, lineno, FileIndex);
                    addtoken(";", lineno, FileIndex);
                    Define(strId, CurrentToken);
                }

                pToken = CurrentToken;
                memset(CurrentToken, 0, sizeof(CurrentToken));
                free(strId);
            }

            else
            {
                addtoken("#", lineno, FileIndex);
                addtoken(";", lineno, FileIndex);
            }

            lineno++;
            continue;
        }

        if (ch == '\n')
        {
            // Add current token..
            addtoken(CurrentToken, lineno++, FileIndex);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }

        // Comments..
        if (ch == '/' && !code.eof())
        {
            bool newstatement = bool( strchr(";{}", CurrentToken[0]) != NULL );

            // Add current token..
            addtoken(CurrentToken, lineno, FileIndex);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;

            // Read next character..
            ch = (char)code.get();

            // If '//'..
            if (ch == '/')
            {
                std::string comment;
                getline( code, comment );   // Parse in the whole comment

                // If the comment says something like "fred is deleted" then generate appropriate tokens for that
                comment = comment + " ";
                if ( newstatement && comment.find(" deleted ")!=std::string::npos )
                {
                    // delete
                    addtoken( "delete", lineno, FileIndex );

                    // fred
                    std::string::size_type pos1 = comment.find_first_not_of(" \t");
                    std::string::size_type pos2 = comment.find(" ", pos1);
                    std::string firstWord = comment.substr( pos1, pos2-pos1 );
                    addtoken( firstWord.c_str(), lineno, FileIndex );

                    // ;
                    addtoken( ";", lineno, FileIndex );
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
            memset(CurrentToken, 0, sizeof(CurrentToken));

            // Read this ..
            CurrentToken[0] = ch;
            CurrentToken[1] = (char)code.get();
            CurrentToken[2] = (char)code.get();
            if (CurrentToken[1] == '\\')
                CurrentToken[3] = (char)code.get();

            // Add token and start on next..
            addtoken(CurrentToken, lineno, FileIndex);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;

            continue;
        }

        // String..
        if (ch == '\"')
        {
            addtoken(CurrentToken, lineno, FileIndex);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            bool special = false;
            char c = ch;
            do
            {
                // Append token..
                if ( pToken < &CurrentToken[sizeof(CurrentToken)-10] )
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
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }

        if (strchr("+-*/%&|^?!=<>[](){};:,.",ch))
        {
            addtoken(CurrentToken, lineno, FileIndex);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            CurrentToken[0] = ch;
            addtoken(CurrentToken, lineno, FileIndex);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }


        if (isspace(ch) || iscntrl(ch))
        {
            addtoken(CurrentToken, lineno, FileIndex);
            pToken = CurrentToken;
            memset(CurrentToken, 0, sizeof(CurrentToken));
            continue;
        }

        *pToken = ch;
        pToken++;
    }

    // Combine tokens..
    for (TOKEN *tok = tokens; tok && tok->next; tok = tok->next)
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
    for ( TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( strcmp(tok->str, "->") == 0 )
        {
            tok->str[0] = '.';
			tok->str[1] = 0;
        }
    }

    // typedef..
    for ( TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if (Match(tok, "typedef %type% %type% ;"))
        {
            const char *type1 = getstr(tok, 1);
            const char *type2 = getstr(tok, 2);
            for ( TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                if (tok2->str!=type1 && tok2->str!=type2 && strcmp(tok2->str,type2)==0)
                {
                    free(tok2->str);
                    tok2->str = _strdup(type1);
                }
            }
        }

        else if (Match(tok, "typedef %type% %type% %type% ;"))
        {
            const char *type1 = getstr(tok, 1);
            const char *type2 = getstr(tok, 2);
            const char *type3 = getstr(tok, 3);
            for ( TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                if (tok2->str!=type3 && strcmp(tok2->str,type3)==0)
                {
                    free(tok2->str);
                    tok2->str = _strdup(type1);

                    TOKEN *newtok = new TOKEN;
                    newtok->str = _strdup(type2);
                    newtok->FileIndex = tok2->FileIndex;
                    newtok->linenr = tok2->linenr;
                    newtok->next = tok2->next;
                    tok2->next = newtok;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// Helper functions for handling the tokens list
//---------------------------------------------------------------------------

const TOKEN *findtoken(const TOKEN *tok1, const char *tokenstr[])
{
    for (const TOKEN *ret = tok1; ret; ret = ret->next)
    {
        unsigned int i = 0;
        const TOKEN *tok = ret;
        while (tokenstr[i])
        {
            if (!tok)
                return NULL;
            if (*(tokenstr[i]) && strcmp(tokenstr[i],tok->str))
                break;
            tok = tok->next;
            i++;
        }
        if (!tokenstr[i])
            return ret;
    }
    return NULL;
}
//---------------------------------------------------------------------------

const TOKEN *gettok(const TOKEN *tok, int index)
{
    while (tok && index>0)
    {
        tok = tok->next;
        index--;
    }
    return tok;
}
//---------------------------------------------------------------------------

const char *getstr(const TOKEN *tok, int index)
{
    tok = gettok(tok, index);
    return tok ? tok->str : "";
}
//---------------------------------------------------------------------------



// Deallocate lists..
void DeallocateTokens()
{
    while (tokens)
    {
        TOKEN *next = tokens->next;
        free(tokens->str);
        delete tokens;
        tokens = next;
    }
    tokens_back = tokens;

    while (dsymlist)
    {
        struct DefineSymbol *next = dsymlist->next;
        free(dsymlist->name);
        free(dsymlist->value);
        delete dsymlist;
        dsymlist = next; 
    }
}




