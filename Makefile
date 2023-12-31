all: BigInt.o test.out asan.out
	size -t BigInt.o test.o

CFLAGS = -Wall -pedantic -Wextra -Werror

BigInt.o: BigInt.cpp BigInt.hpp
	g++ $(CFLAGS) -Os -s -c BigInt.cpp -o $@

test.o: BigInt.cpp BigInt.hpp
	g++ $(CFLAGS) -Os -g -c BigInt.cpp -o $@

asan.o: BigInt.cpp BigInt.hpp
	g++ $(CFLAGS) -Os -g -fsanitize=address -c BigInt.cpp -o $@


test.out: test.cpp test.o BigInt.hpp
	g++ $(CFLAGS) -Os -g test.cpp test.o -o $@

asan.out: test.cpp asan.o BigInt.hpp
	g++ $(CFLAGS) -Os -g -fsanitize=address test.cpp asan.o -o $@
