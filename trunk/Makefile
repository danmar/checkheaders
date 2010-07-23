SRCS=checkheaders.cpp  commoncheck.cpp  tokenize.cpp
OBJS=$(SRCS:%.cpp=%.o)

%.o:	%.cpp
	g++ -Wall -pedantic -g -I. -o $@ -c $^

all:	${OBJS} main.o
	g++ -Wall -g -o checkheaders $^

clean:
	rm -f *.o checkheaders
