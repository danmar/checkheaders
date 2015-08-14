Check headers in C/C++ programs (detect unnecessary includes).

# News #
  * 2010-11-20 : Released version 1.0.1 - A few minor updates ([issue 12](https://code.google.com/p/checkheaders/issues/detail?id=12)) ([issue 13](https://code.google.com/p/checkheaders/issues/detail?id=13)) ([issue 14](https://code.google.com/p/checkheaders/issues/detail?id=14)).
  * 2010-08-12 : Released version 1.0 - Checkheaders is now as fast, stable and accurate as it needs to be for the 1.0 release. Compared to version 0.3 there are just a few bug fixes.
  * 2010-07-30 : Released version 0.3 - Check system headers.
  * 2010-07-27 : Released version 0.2 - Fixed false positives ([issue 2](https://code.google.com/p/checkheaders/issues/detail?id=2)) ([issue 3](https://code.google.com/p/checkheaders/issues/detail?id=3)). Recursive checking when given a folder. Added Visual Studio project.
  * 2010-07-24 : Released version 0.1

# How it works #

A #include is needed if any symbol names match. If there is no matching symbol names, it is not needed.

This tool will work best if all classes/variables/constants/functions/etc have unique names.

# Status #

This project is finished but I will continue to fix bugs.

If you would like to take over the project then contact me.