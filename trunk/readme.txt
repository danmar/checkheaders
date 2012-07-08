

============
checkheaders
============

Purpose

  A simple tool that checks headers, to detect unnecessary #includes.

Compiling

  Any C++ compiler should work. 
  There are no dependencies.

  The compilation system is based on cmake and should work on all OSs supported by cmake.
 
  In order to compile follow these steps:
   * create a folder named 'build' under the root folder where the sources are located
   * change the current folder to 'build' folder
   * use either 'cmake ..' (for Linux) or cmake GUI (for Windows). Check cmake help
for more options.
   * compile with either 'make' or Visual Studio C++
   * optionally install with 'make install' (Linux only)  

Usage

  The syntax is:
      checkheaders [-I <path>] [--skip <file>] [--file <file>] [--xml] <path or file>

  The error messages will be printed to stderr.

  Examples:
      checkheaders path
      checkheaders f1.c f2.c

Project home

  http://code.google.com/p/checkheaders
