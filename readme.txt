

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
      checkheaders [-I <path>] [--skip <file>] [--skip-all] [--file <file>] [--xml] [--quiet] <path or file>

Options

  -I             Include path
  --file <file>  Specify the files to check in a text file 
  --quiet        Do not show progress
  --skip <file>  Skip missing include file
  --skip-all     Skip all missing include files 
  --version      Print out version number
  --vs           Output report in VisualStudio format 
  --xml          Output report in XML format 
  
  The error messages will be printed to stderr.

  Examples:
      checkheaders path
      checkheaders f1.c f2.c

Project home

  http://code.google.com/p/checkheaders
