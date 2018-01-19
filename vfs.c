#define _GNU_SOURCE
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

/* implementation eventually delegated to the client program */

int should_fake_file(const char *pathname) {
	return (!strncmp(pathname, "/fake/", 6));
}

int should_fake_dir(const char *pathname) {
	return (!strncmp(pathname, "/fake/dir", 9));
}

char* generate_file_contents(const char *pathname) {
	// no asprintf in this QNX :(
	const char fmt_str_1[] = "tricked you! `";
	const char fmt_str_2[] = "` doesn't exist!";
	char *full = malloc(strlen(fmt_str_1) + strlen(fmt_str_2) + strlen(pathname) + 1);
	strcpy(full, fmt_str_1);
	strcpy(full + strlen(fmt_str_1), pathname);
	strcpy(full + strlen(fmt_str_1) + strlen(pathname), fmt_str_2);

	return full;
}

char** generate_folder_contents(const char *pathname) {
	char **res = malloc(sizeof(char*[3]));
	res[0] = "file1";
	res[1] = "file2";
	res[2] = NULL;
	return res;
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
		char *contents = generate_file_contents(pathname);
		sprintf(ff->buf, "fd %d: %s\n", ff->fd, contents);
		free(contents);
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
	unlink_ff(ff);
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
	char* contents = generate_file_contents(pathname);
	fprintf(fp, "FILE* from fd %d: %s\n", fd, contents);
	free(contents);
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

/* directory functions */

int is_fake_dir(DIR *dirp) {
	return lookup_dirp(dirp) != NULL;
}

DIR *open_fake_dir(const char *pathname) {
	return NULL;
}

struct dirent *read_fake_dir(DIR *dirp) {
	return NULL;
}

void rewind_fake_dir(DIR *dirp) {
	return;
}

int close_fake_dir(DIR *dirp) {
	return -1;
}
