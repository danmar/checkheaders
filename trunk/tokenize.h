/*
 * checkheaders - check headers in C/C++ code
 * Copyright (C) 2010 Daniel Marjam�ki.
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

#include <string>
#include <vector>

extern std::vector<std::string> Files;

struct TOKEN
{
    unsigned int FileIndex;
    char *str;
    unsigned int linenr;
    struct TOKEN *next;
};
extern struct TOKEN *tokens, *tokens_back;


void Tokenize(const char FileName[]);

void TokenizeCode(std::istream &code, const unsigned int FileIndex=0);

// Deallocate lists..
void DeallocateTokens();

// Helper functions for handling the tokens list..
const TOKEN *findtoken(const TOKEN *tok1, const char *tokenstr[]);
const TOKEN *gettok(const TOKEN *tok, int index);
const char *getstr(const TOKEN *tok, int index);


//---------------------------------------------------------------------------
#endif

