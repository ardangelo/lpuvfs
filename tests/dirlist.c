#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

int main(int argc, char **argv) {
	DIR *d = opendir(argv[1]);
	printf("reading dir: %s\n", argv[1]);
	for (struct dirent *p = readdir(d); p != NULL; p = readdir(d)) {
		printf("%s\n", p->d_name);
	}
	closedir(d);
}
