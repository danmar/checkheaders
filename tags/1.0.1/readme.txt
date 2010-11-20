

============
checkheaders
============

Purpose

  A simple tool that checks headers, to detect unnecessary #includes.

Compiling

  Any C++ compiler should work. 
  There are no dependencies.

  The Makefile is written for g++.
   * To compile in Linux a simple "make" will work.
   * To compile in DOS/Windows with DJGPP, use "make CXX=gxx"
   * To compile in Windows with MINGW, use "make LDFLAGS=-lshlwapi"

  There are also Visual Studio project/solution files.

Usage

  The syntax is:
      checkheaders [-I <path>] [--skip <file>] [--xml] <path or file>

  The error messages will be printed to stderr.

  Examples:
      checkheaders path
      checkheaders f1.c f2.c

Project home

  http://code.google.com/p/checkheaders
