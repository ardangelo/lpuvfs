.PHONY: all tests run-tests

TEST_SRCS := $(wildcard tests/*.c)
TESTS := $(TEST_SRCS:.c=.out)

CFLAGS := -g -std=c99

all: lpuvfs.so $(TESTS)

vfs.o: vfs.c
	gcc $(CFLAGS) -c vfs.c -o vfs.o
int.o: int.c
	gcc $(CFLAGS) -c int.c -o int.o
lpuvfs.so: int.o vfs.o
	gcc -shared -fPIC int.o vfs.o -o lpuvfs.so

%.out: %.c
	gcc $(CFLAGS) $< -o $@
run-tests: $(TESTS) lpuvfs.so
	LD_PRELOAD=$(PWD)/lpuvfs.so tests/open.out /fake/test
	LD_PRELOAD=$(PWD)/lpuvfs.so tests/fopen.out /fake/test

clean:
	rm *.so *.o
	rm $(TESTS)
