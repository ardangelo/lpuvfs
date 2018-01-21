.PHONY: all tests run-tests

TEST_SRCS := $(wildcard tests/*.c)
TESTS := $(TEST_SRCS:.c=.out)

CFLAGS := -g -std=c99

all: lpuvfs.so $(TESTS)

ll.o: ll.c
	gcc $(CFLAGS) -c ll.c -o ll.o
vfs.o: vfs.c
	gcc $(CFLAGS) -c vfs.c -o vfs.o
int.o: int.c
	gcc $(CFLAGS) -c int.c -o int.o
lpuvfs.so: int.o vfs.o ll.o
	gcc -shared -fPIC int.o vfs.o ll.o -o lpuvfs.so

%.out: %.c
	gcc $(CFLAGS) $< -o $@

find-syms:
	LD_DEBUG=all $(CMD) 2>&1 | grep 'Looking up' | grep -v ' _'

run-cmd: lpuvfs.so
	LD_PRELOAD=$(PWD)/lpuvfs.so $(CMD)	

run-tests: $(TESTS) lpuvfs.so
	LD_PRELOAD=$(PWD)/lpuvfs.so tests/open.out /fake/test1 /fake/test2
	LD_PRELOAD=$(PWD)/lpuvfs.so tests/fopen.out /fake/test
	LD_PRELOAD=$(PWD)/lpuvfs.so tests/dirlist.out /fake/

clean:
	rm *.so *.o
	rm $(TESTS)
