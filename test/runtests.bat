
del testrunner.exe

copy ..\*.cpp .
del main.cpp

copy ..\*.h .

gxx -o testrunner.exe *.cpp

testrunner