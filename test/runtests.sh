#/bin/bash

rm testrunner
g++ -I.. -o testrunner testrunner.cpp testsuite.cpp testwarningincludeheaders.cpp ../checkheaders.cpp ../commoncheck.cpp ../tokenize.cpp
./testrunner

