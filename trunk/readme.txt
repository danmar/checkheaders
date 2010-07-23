

============
checkheaders
============

Compiling

  Any C++ compiler should work. 
  There are no dependencies.

  The Makefile works under Linux.
  To make it work under Windows, change "g++" to "gxx".

Usage

  The syntax is:
      checkheaders [filename1] [filename2]

  The error messages will be printed to stderr.

  Examples:
      checkheaders *.cpp
      checkheaders f1.c f2.c


Recommendations

  To dump the messages to a textfile you can use a command like this:
      checkheaders filename.cpp 2> messages.txt

  If you want to filter the messages you could use:
    * grep to filter out specific types of messages
    * diff to compare old messages with new messages.

Project home

  http://code.google.com/p/checkheaders/

Author

  Daniel Marjamäki

