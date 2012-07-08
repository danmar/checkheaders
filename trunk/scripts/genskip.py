#!/usr/bin/env python

#print to standard output a list of skip files

import sys
import re

if 2 != len(sys.argv):
    print "one aregument required", sys.argv[0], "info.txt"
    sys.exit()

f = open(sys.argv[1])

hdrs = []
for line in f:
    if "Header not found" in line:
        m = re.search('(?<=\')[\w\.\/]+', line)
        found = m.group(0)
        if not (found in hdrs):
            hdrs.append(found)

hdrs.sort()
print "skip\n"
for found in hdrs:
    print found

