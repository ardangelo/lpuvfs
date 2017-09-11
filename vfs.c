#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* memory management */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* file control */
#include <fcntl.h>

#include "vfs.h"

typedef struct _fake_file_t {
	char *buf;
	size_t size;
	int fd;
	FILE *fp;
	const char *name;
} fake_file_t;

const char str[] = "tricked you!";
fake_file_t *ff = NULL;

int should_fake(const char *pathname) {
	return (!strncmp(pathname, "/fake/", 6));
}

/* fd functions */

int is_fake_fd(int fd) {
	return (ff->fd == fd);
}

int open_fake_fd(const char *pathname) {
	if (ff != NULL) {
		/* no multi-file support yet */
		return -1;
	}
	ff = malloc(sizeof(fake_file_t));
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
	munmap(ff->buf, ff->size);
	shm_unlink(ff->name);
	free(ff);
	ff = NULL;
	return 0;
}

/* FILE* functions */

int is_fake_file(FILE *fp) {
	return (ff->fp == fp);
}

FILE *open_fake_file(const char *path) {
	int fd = open_fake_fd(path);
	FILE *fp = fdopen(fd, "w+");
	fprintf(fp, "FILE* from fd %d: %s\n", fd, str);
	rewind(fp);

	return fp;
}

int close_fake_file(FILE *fp) {
	fclose(ff->fp);
	free(ff->buf);
	close_fake_fd(ff->fd);
}
