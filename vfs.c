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

char *membuf = NULL;
size_t memsize = 0;
int memfd = -1;
FILE *memfp = NULL;
const char *memname = "/faker";

const char str[] = "tricked you!\n";

int should_fake(const char *pathname) {
	return (!strncmp(pathname, "/fake/", 6));
}

/* fd functions */

int is_fake_fd(int fd) {
	return (memfd == fd);
}

int open_fake_fd(const char *pathname) {
	if (membuf != NULL) {
		/* no multi-file support yet */
		return -1;
	}

	memsize = getpagesize();
	memfd = shm_open(memname, O_CREAT | O_RDWR, 0777);

	if (memfd > 0) {
		ftruncate(memfd, memsize);
		membuf = (char*)mmap(0, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
		strcpy(membuf, str);
	}

	return memfd;
}

int close_fake_fd(int fd) {
	munmap(membuf, memsize);
	shm_unlink(memname);
	memfd = -1;
	membuf = NULL;
	memsize = 0;
	return 0;
}

/* FILE* functions */

int is_fake_file(FILE *fp) {
	return (memfp == fp);
}

FILE *open_fake_file(const char *path) {
	if (membuf != NULL) {
		/* no multi-file support yet */
		return NULL;
	}

	memsize = getpagesize();
	FILE *fp = fmemopen((void*)membuf, memsize, "w+");
	fprintf(fp, "%s", str);
	rewind(fp);

	return fp;
}

int close_fake_file(FILE *fp) {
	fclose(fp);
}
