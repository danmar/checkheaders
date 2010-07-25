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

// TODO: Remove this
bool XmlOutput;

class TestWarningIncludeHeaders : public TestFixture
{
public:
    TestWarningIncludeHeaders() : TestFixture("TestWarningIncludeHeaders")
    {
        XmlOutput = false;
    }

private:
    void run()
    {
        TEST_CASE(issue3);
        TEST_CASE(test1);
    }

    void test1()
    {
        {
            std::ofstream f1("a.c");
            f1 << "#include \"a.h\"\n"
               << "int i[NUM];\n";

            std::ofstream f2("a.h");
            f2 << "#include \"fred.h\"\n"
               << "const int NUM = 10;\n";

            std::ofstream f3("fred.h");
            f3 << "class Fred { };\n";
        }

        tokens = tokens_back = NULL;
        Files.clear();
        Tokenize("a.c");

        // Including header which is not needed
        std::ostringstream errout;
        WarningIncludeHeader(errout);

        // Clean up tokens..
        DeallocateTokens();

        ASSERT_EQUALS("[a.h:1] (style): The included header 'fred.h' is not needed\n", errout.str());
    }

    void issue3()
    {
        {
            std::ofstream f1("a.c");
            f1 << "#include \"fred.h\"\n"
               << "struct APP_INIT_DATA {\n"
               << "  PROXY_INFO proxy_info;\n"
               << "};\n";

            std::ofstream f2("a.h");
            f2 << "struct PROXY_INFO { bool use_http_proxy; };\n";
        }

        tokens = tokens_back = NULL;
        Files.clear();
        Tokenize("a.c");

        // Including header which is not needed
        std::ostringstream errout;
        WarningIncludeHeader(errout);

        // Clean up tokens..
        DeallocateTokens();

        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestWarningIncludeHeaders)
