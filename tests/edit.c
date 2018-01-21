#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

int main (int argc, char **argv) {
	FILE *fp = fopen(argv[1], "w");
	int fd = fileno(fp);

	struct stat buf;
	fstat(fd, buf);
	printf("atime pre: %d\n", buf.st_mtime);

	fprintf(fp, "Line 2\n");

	fstat(fd, buf);
	printf("atime post: %d\n", buf.st_mtime);

	fclose(fp);

	return 0;
}
