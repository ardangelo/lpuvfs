.PHONY: all test
all: int.so reader.out

int.so: int.c
	gcc -shared -fPIC -g int.c -o int.so

reader.out:
	gcc -g reader.c -o reader.out

test: reader.out
	LD_PRELOAD=$(PWD)/int.so ./reader.out /fake/test

clean:
	rm *.so *.out
