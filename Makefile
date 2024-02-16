all: BigInt.a test.out
	size -t BigInt.a test.out

CFLAGS = -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color -std=c++11

BigInt.a: build/impl.o
	ar rcu $@ $<

build/impl.o: src/impl.cpp include/BigInt.hpp
	g++ $(CFLAGS) -o $@ -Os -s -c $<

test.out: test.cpp src/impl.cpp include/BigInt.hpp
	g++ $(CFLAGS) -o $@ -Os -g -fsanitize=address test.cpp src/impl.cpp
