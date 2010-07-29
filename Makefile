SRCS=checkheaders.cpp	commoncheck.cpp	filelister.cpp	tokenize.cpp
OBJS=$(SRCS:%.cpp=%.o)
HDRS=$(SRCS:%.cpp=%.h)
CXX=g++
CXXFLAGS=-g
APPNAME=checkheaders

%.o:	%.cpp	$(HDRS)
	$(CXX) -Wall -pedantic $(CXXFLAGS) -I. -o $@ -c $<

all:	${OBJS}	main.o
	$(CXX) -Wall $(CXXFLAGS) -o ${APPNAME} $^

test:	all
	$(CXX) -I. -Wall $(CXXFLAGS) -o test/testrunner ${OBJS} test/*.cpp
	cd test;./testrunner;cd ..

install:
	install -d /usr/bin
	install ${APPNAME} /usr/bin

clean:
	rm -f *.o ${APPNAME}

