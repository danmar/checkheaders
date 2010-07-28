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

#include "tokenize.h"
#include "checkheaders.h"
#include "testsuite.h"
#include <fstream>
#include <sstream>
#include <vector>

class TestWarningIncludeHeaders : public TestFixture
{
public:
    TestWarningIncludeHeaders() : TestFixture("TestWarningIncludeHeaders")
    { }

private:
    const std::vector<std::string> includePaths;

    void run()
    {
        TEST_CASE(implementation1);
        TEST_CASE(implementation2);
        TEST_CASE(issue3);
        TEST_CASE(needed_define);
        TEST_CASE(needed_typedef);
        TEST_CASE(stdafx);
        TEST_CASE(test1);
    }

    void implementation1()
    {
        // Call function in header
        {
            std::ofstream f1("implementation1.c");
            f1 << "#include \"implementation1.h\"\n"
               << "void f()\n"
               << "{\n"
               << "    hello();\n"
               << "}\n";

            std::ofstream f2("implementation1.h");
            f2 << "void hello()\n"
               << "{\n"
               << "}\n";
        }

        std::ostringstream errout;
        
        Tokenizer tokenizer;
        tokenizer.tokenize("implementation1.c", includePaths, false, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, errout);

        ASSERT_EQUALS("", errout.str());
    }
    
    void implementation2()
    {
        // header not needed
        {
            std::ofstream f1("implementation2.c");
            f1 << "#include \"implementation2.h\"\n"
               << "void f()\n"
               << "{\n"
               << "}\n";

            std::ofstream f2("implementation2.h");
            f2 << "void hello()\n"
               << "{\n"
               << "}\n";
        }
        
        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("implementation2.c", includePaths, false, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, errout);

        ASSERT_EQUALS("[implementation2.c:1] (style): The included header 'implementation2.h' is not needed\n", errout.str());
    }

    void issue3()
    {
        {
            std::ofstream f1("issue3.c");
            f1 << "#include \"issue3.h\"\n"
               << "struct APP_INIT_DATA {\n"
               << "  PROXY_INFO proxy_info;\n"
               << "};\n";

            std::ofstream f2("issue3.h");
            f2 << "struct PROXY_INFO { bool use_http_proxy; };\n";
        }

        std::ostringstream errout;
        
        Tokenizer tokenizer;
        tokenizer.tokenize("issue3.c", includePaths, false, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void needed_define()
    {
        {
            std::ofstream f1("needed_define.c");
            f1 << "#include \"needed_define.h\"\n"
               << "void foo() __attribute__((deprecated));\n";

            std::ofstream f2("needed_define.h");
            f2 << "#define __attribute__(x)\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("needed_define.c", includePaths, false, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void needed_typedef()
    {
        {
            std::ofstream f1("needed_typedef.c");
            f1 << "#include \"needed_typedef.h\"\n"
               << "U32 foo()\n"
               << "{ return 0; }";

            std::ofstream f2("needed_typedef.h");
            f2 << "typedef unsigned int U32;\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("needed_typedef.c", includePaths, false, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void stdafx()
    {
        {
            std::ofstream f1("stdafx.c");
            f1 << "#include \"stdafx.h\"\n";    // stdafx.h is always needed

            std::ofstream f2("stdafx.h");
            f2 << "#include <stdio.h>\n";       // stdafx.h needs all included headers
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("stdafx.c", includePaths, false, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, errout);

        ASSERT_EQUALS("", errout.str());
    }
    
    void test1()
    {
        {
            std::ofstream f1("test1.c");
            f1 << "#include \"test1.h\"\n";

            std::ofstream f2("test1.h");
            f2 << "class Fred { };\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("test1.c", includePaths, false, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, errout);

        ASSERT_EQUALS("[test1.c:1] (style): The included header 'test1.h' is not needed\n", errout.str());
    }
};

REGISTER_TEST(TestWarningIncludeHeaders)
