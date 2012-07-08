#!/usr/bin/env python

#print to standard output a list of skip files

import sys

if 2 != len(sys.argv):
    print "one aregument required", sys.argv[0], "info.txt"
    sys.exit()

f = open(sys.argv[1])

for line in f:
    if "is not needed" in line:
        print line

