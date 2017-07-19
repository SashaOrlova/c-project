all: test

test: ./test.o
	g++ ./test.o -lcds -lpthread -lgtest -o test

./test.o: ./test.cpp
	g++ -std=c++14 -g -I../../libcds -c -o $@ $<

clean:
	rm -rf test test.o

.PHONY: clean all
