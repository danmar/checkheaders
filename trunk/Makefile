SRCS=src/checkheaders.cpp	src/commoncheck.cpp	src/filelister.cpp	src/tokenize.cpp
OBJS=$(SRCS:%.cpp=%.o)
HDRS=$(SRCS:%.cpp=%.h)
CXX=g++
CXXFLAGS=-g
APPNAME=checkheaders

%.o:	%.cpp	$(HDRS)
	$(CXX) -Wall -pedantic $(CXXFLAGS) -I. -o $@ -c $<

all:	${OBJS}	src/main.o
	$(CXX) -Wall $(CXXFLAGS) -o ${APPNAME} $^

testrunner:	all
	$(CXX) -I src -Wall $(CXXFLAGS) -o test/testrunner ${OBJS} test/*.cpp
	
test:	testrunner
	cd test;./testrunner;cd ..

install:
	install -d /usr/bin
	install ${APPNAME} /usr/bin

clean:
	rm -f src/*.o test/*.o ${APPNAME}
