/* fread example: read an entire file */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main (int argc, char **argv) {
	int fd = open(argv[1], O_RDONLY);
	char buf[65];

	printf ("File handle %d\n", fd);
	
	int r = 0;
	while ((r = read(fd, buf, 64)) > 0) {
		buf[r] = '\0';
		printf(buf);
	}

	return 0;
}
