SRCS=checkheaders.cpp	commoncheck.cpp	filelister.cpp	tokenize.cpp
OBJS=$(SRCS:%.cpp=%.o)
CXX=g++
APPNAME=checkheaders

%.o:	%.cpp
	$(CXX) -Wall -pedantic -g -I. -o $@ -c $^

all:	${OBJS}	main.o
	$(CXX) -Wall -g -o ${APPNAME} $^

test:	all
	$(CXX) -I. -Wall -g -o test/testrunner ${OBJS} test/*.cpp
	cd test;./testrunner;cd ..

install:
	install -d /usr/bin
	install ${APPNAME} /usr/bin

clean:
	rm -f *.o ${APPNAME}

