SRCS=checkheaders.cpp  commoncheck.cpp  tokenize.cpp
OBJS=$(SRCS:%.cpp=%.o)
CXX=g++

%.o:	%.cpp
	$(CXX) -Wall -pedantic -g -I. -o $@ -c $^

all:	${OBJS} main.o
	$(CXX) -Wall -g -o checkheaders $^

clean:
	rm -f *.o checkheaders
