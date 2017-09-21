/* fread example: read an entire file */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main (int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		int fd = open(argv[i], O_RDONLY);
		char buf[65];

		printf ("File handle %d\n", fd);

		int r = 0;
		while ((r = read(fd, buf, 64)) > 0) {
			buf[r] = '\0';
			printf(buf);
		}

		close(fd);
	}

	return 0;
}
