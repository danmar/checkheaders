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

class TestWarningIncludeHeaders : public TestFixture
{
public:
    TestWarningIncludeHeaders() : TestFixture("TestWarningIncludeHeaders")
    { }

private:
    void run()
    {
        TEST_CASE(issue3);
        TEST_CASE(needed_define);
        TEST_CASE(test1);
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

        const Tokenizer tokenizer("issue3.c");

        // Including header which is not needed
        std::ostringstream errout;
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

        const Tokenizer tokenizer("needed_define.c");

        // Including header which is not needed
        std::ostringstream errout;
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

        const Tokenizer tokenizer("test1.c");

        // Including header which is not needed
        std::ostringstream errout;
        WarningIncludeHeader(tokenizer, false, errout);

        ASSERT_EQUALS("[test1.c:1] (style): The included header 'test1.h' is not needed\n", errout.str());
    }
};

REGISTER_TEST(TestWarningIncludeHeaders)
