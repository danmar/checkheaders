

============
checkheaders
============

Purpose

  A simple tool that checks headers, to detect unnecessary #includes.

Compiling

  Any C++ compiler should work. 
  There are no dependencies.

  The Makefile is written for g++.

  The "checkheaders.vcproj" is a Visual Studio project
  created with Visual Studio 2008 Express.

Usage

  The syntax is:
      checkheaders [-I <path>] [--skip <file>] [--xml] <path or file>

  The error messages will be printed to stderr.

  Examples:
      checkheaders path
      checkheaders f1.c f2.c

Project home

  http://code.google.com/p/checkheaders
