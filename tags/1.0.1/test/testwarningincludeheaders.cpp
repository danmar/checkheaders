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
    const std::set<std::string> skipIncludes;

    void run()
    {
        TEST_CASE(declaration1);
        TEST_CASE(declaration2);
        TEST_CASE(implementation1);
        TEST_CASE(implementation2);
        TEST_CASE(indentlevel);
        TEST_CASE(issue3);
        TEST_CASE(needed_class);
        TEST_CASE(needed_const);
        TEST_CASE(needed_define1);
        TEST_CASE(needed_define2);
        TEST_CASE(needed_include);
        TEST_CASE(needed_typedef);
        TEST_CASE(needed_namespace);
        TEST_CASE(stdafx);
        TEST_CASE(standardheader1);
        TEST_CASE(standardheader2);
        TEST_CASE(test1);
    }

    void declaration1()
    {
        // Header is not needed
        {
            std::ofstream f1("declaration1.c");
            f1 << "#include \"declaration1.h\"\n"
               << "void f(Fred *fred);\n";

            std::ofstream f2("declaration1.h");
            f2 << "struct Fred\n"
               << "{\n"
               << "};\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("declaration1.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("[declaration1.c:1] (style): The included header 'declaration1.h' is not needed (but forward declaration is needed)\n", errout.str());
    }

    void declaration2()
    {
        // Header is not needed
        {
            std::ofstream f1("declaration2.c");
            f1 << "#include \"declaration2.h\"\n"
               << "Foo *foo;\n"
               << "void f()\n"
               << "{\n"
               << "    foo->x();\n"
               << "}\n";

            std::ofstream f2("declaration2.h");
            f2 << "struct Foo\n"
               << "{\n"
               << "    void x();\n"
               << "};\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("declaration2.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("", errout.str());
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
        tokenizer.tokenize("implementation1.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

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
        tokenizer.tokenize("implementation2.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("[implementation2.c:1] (style): The included header 'implementation2.h' is not needed\n", errout.str());
    }

    void indentlevel()
    {
        // the indentlevel will be increased too much in the included header.
        {
            std::ofstream f1("indentlevel.c");
            f1 << "#include \"indentlevel.h\"\n"
               << "char a[NUM];\n";

            std::ofstream f2("indentlevel.h");
            f2 << "#include <indentlevel-list.h>\n"
               << "const int NUM = 10;\n";

            std::ofstream f3("indentlevel-list.h");
            f3 << "void foo()\n"
               << "{\n"
               << "#ifdef AAA\n"
               << "    if (aaa) {\n"
               << "#else\n"
               << "    if (bbb) {\n"
               << "#endif\n"
               << "    }\n"
               << "}\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("indentlevel.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("[indentlevel.h:1] (style): The included header 'indentlevel-list.h' is not needed\n", errout.str());
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
        tokenizer.tokenize("issue3.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void needed_class()
    {
        {
            std::ofstream f1("needed_class.cpp");
            f1 << "#include \"needed_class.h\"\n"
               << "#include \"needed_class-ab.h\"\n"
               << "void Foo::f() { ab->do_something(); }\n";

            std::ofstream f2("needed_class.h");
            f2 << "struct AB;\n"
               << "struct Foo {\n"
               << "    AB *ab;\n"
               << "    void f();\n"
               << "};";

            std::ofstream f3("needed_class-ab.h");
            f3 << "struct AB {\n"
               << "    void do_something() { }\n"
               << "};";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("needed_class.cpp", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void needed_const()
    {
        {
            std::ofstream f1("needed_const.c");
            f1 << "#include \"needed_const.h\"\n"
               << "void foo() { char a[10]; a[DEFAULT_LANGUAGE] = 0; }\n";

            std::ofstream f2("needed_const.h");
            f2 << "const int DEFAULT_LANGUAGE = 0;\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("needed_const.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void needed_define1()
    {
        {
            std::ofstream f1("needed_define1.c");
            f1 << "#include \"needed_define1.h\"\n"
               << "void foo() __attribute__((deprecated));\n";

            std::ofstream f2("needed_define1.h");
            f2 << "#define __attribute__(x)\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("needed_define1.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void needed_define2()
    {
        {
            std::ofstream f1("needed_define2.c");
            f1 << "#include \"needed_define2.h\"\n"
               << "void f() { b(1,XY,2); }\n";

            std::ofstream f2("needed_define2.h");
            f2 << "extern \"C\" {"
               << "#define XY 0x033\n"
               << "}";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("needed_define2.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void needed_include()
    {
        {
            std::ofstream f1("needed_include.c");
            f1 << "#include \"needed_include1.h\"\n"
               << "void f() { b(); }\n";

            std::ofstream f2("needed_include1.h");
            f2 << "#include \"needed_include2.h\"\n";

            std::ofstream f3("needed_include2.h");
            f3 << "void b();\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("needed_include.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("[needed_include.c:1] (style): Inconclusive results: The included header 'needed_include1.h' is not needed. "
                      "However it is needed indirectly because it includes 'needed_include2.h'. "
                      "If it is included by intention use '--skip needed_include1.h' to remove false positives.\n"
                      "[needed_include1.h:1] (style): The included header 'needed_include2.h' is not needed\n", errout.str());
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
        tokenizer.tokenize("needed_typedef.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void needed_namespace()
    {
        {
            std::ofstream f1("needed_namespace.c");
            f1 << "#include \"needed_namespace.h\"\n"
               << "void foo()\n"
               << "{ foo::bar foobar; }";

            std::ofstream f2("needed_namespace.h");
            f2 << "namespace foo {\n"
               << "    class bar { };\n"
               << "}\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("needed_namespace.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

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
        tokenizer.tokenize("stdafx.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("", errout.str());
    }

    void standardheader1()
    {
        {
            std::ofstream f1("standardheader1.c");
            f1 << "#include <standardheader1.h>\n";

            std::ofstream f2("standardheader1.h");
            f2 << "void foo();\n";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("standardheader1.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("[standardheader1.c:1] (style): The included header 'standardheader1.h' is not needed\n", errout.str());
    }

    void standardheader2()
    {
        {
            std::ofstream f1("standardheader2.c");
            f1 << "#include <standardheader2.h>\n"
               << "void f()\n"
               << "{ x(); }";

            std::ofstream f2("standardheader2.h");
            f2 << "#include <x.h>\n";

            std::ofstream f3("x.h");
            f3 << "void x();";
        }

        std::ostringstream errout;

        Tokenizer tokenizer;
        tokenizer.tokenize("standardheader2.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

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
        tokenizer.tokenize("test1.c", includePaths, skipIncludes, OUTPUT_FORMAT_NORMAL, errout);

        // Including header which is not needed
        WarningIncludeHeader(tokenizer, false, OUTPUT_FORMAT_NORMAL, errout);

        ASSERT_EQUALS("[test1.c:1] (style): The included header 'test1.h' is not needed\n", errout.str());
    }
};

REGISTER_TEST(TestWarningIncludeHeaders)
