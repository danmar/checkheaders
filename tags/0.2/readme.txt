

============
checkheaders
============

Compiling

  Any C++ compiler should work. 
  There are no dependencies.

  The Makefile is written for g++.

  The "checkheaders.vcproj" is a Visual Studio project
  created with Visual Studio 2008 Express.

Usage

  The syntax is:
      checkheaders [path] [filename1] [filename2]

  The error messages will be printed to stderr.

  Examples:
      checkheaders path
      checkheaders f1.c f2.c


Recommendations

  To dump the messages to a textfile you can use a command like this:
      checkheaders filename.cpp 2> messages.txt

Project home

  http://code.google.com/p/checkheaders
