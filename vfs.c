#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* memory management */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* file control */
#include <fcntl.h>

#include "ll.h"
#include "vfs.h"

const char str[] = "tricked you!";

int should_fake(const char *pathname) {
	return (!strncmp(pathname, "/fake/", 6));
}

/* fd functions */

int is_fake_fd(int fd) {
	return lookup_fd(fd) != NULL;
}

int open_fake_fd(const char *pathname) {
	fake_file_t *ff = lookup_name(pathname);
	if (ff == NULL) {
	    ff = new_ff();
	} else {
	    return ff->fd;
	}

	ff->size = getpagesize();
	ff->name = pathname;
	ff->fd = shm_open(ff->name, O_CREAT | O_RDWR, 0777);

	if (ff->fd > 0) {
		ftruncate(ff->fd, ff->size);
		ff->buf = (char*)mmap(0, ff->size, PROT_READ | PROT_WRITE, MAP_SHARED, ff->fd, 0);
		sprintf(ff->buf, "fd %d: %s\n", ff->fd, str);
	}

	return ff->fd;
}

int close_fake_fd(int fd) {
    fake_file_t *ff = lookup_fd(fd);
	if (ff == NULL) {
	    return -1;
	}

	munmap(ff->buf, ff->size);
	shm_unlink(ff->name);
	free(ff);
	ff = NULL;
	return 0;
}

/* FILE* functions */

int is_fake_file(FILE *fp) {
	return lookup_fp(fp) != NULL;
}

FILE *open_fake_file(const char *pathname) {
    fake_file_t *ff = lookup_name(pathname);
	if (ff != NULL) {
	    return ff->fp;
	}

	int fd = open_fake_fd(pathname);
	FILE *fp = fdopen(fd, "w+");
	fprintf(fp, "FILE* from fd %d: %s\n", fd, str);
	rewind(fp);

	return fp;
}

int close_fake_file(FILE *fp) {
    fake_file_t *ff = lookup_fp(fp);
	if (ff == NULL) {
	    return -1;
	}

	fclose(ff->fp);
	free(ff->buf);
	close_fake_fd(ff->fd);
}
