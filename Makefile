.PHONY: all tests run-tests
all: lpuvfs.so tests

vfs.o: vfs.c
	gcc -g -c vfs.c -o vfs.o

int.o: int.c
	gcc -g -c int.c -o int.o

lpuvfs.so: int.o vfs.o
	gcc -shared -fPIC -g int.o vfs.o -o lpuvfs.so

tests: open.c fopen.c
	gcc -g open.c -o open.out
	gcc -g fopen.c -o fopen.out

run-tests: tests lpuvfs.so
	LD_PRELOAD=$(PWD)/lpuvfs.so ./open.out /fake/test

clean:
	rm *.so *.out *.o
