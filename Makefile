.PHONY: all tests run-tests

TEST_SRCS := $(wildcard tests/*.c)
TESTS := $(TEST_SRCS:.c=.out)

all: lpuvfs.so $(TESTS)

vfs.o: vfs.c
	gcc -g -c vfs.c -o vfs.o
int.o: int.c
	gcc -g -c int.c -o int.o
lpuvfs.so: int.o vfs.o
	gcc -shared -fPIC -g int.o vfs.o -o lpuvfs.so

%.out: %.c
	gcc -g $< -o $@
run-tests: $(TESTS) lpuvfs.so
	LD_PRELOAD=$(PWD)/lpuvfs.so tests/open.out /fake/test

clean:
	rm *.so *.o
	rm $(TESTS)
