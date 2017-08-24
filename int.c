#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* dynamic linking */
#include <dlfcn.h>

/* file control */
#include <fcntl.h>
#include <unistd.h>

#include "vfs.h"

/* fd functions */
typedef int (*orig_open_t)(const char *pathname, int flags, ...);
typedef int (*orig_creat_t)(const char *pathname, mode_t mode);
typedef ssize_t (*orig_read_t)(int fd, void *buf, size_t count);
typedef ssize_t (*orig_write_t)(int fd, const void *buf, size_t count);
typedef int (*orig_unlink_t)(const char *pathname);
typedef int (*orig_close_t)(int fd);

int open(const char *pathname, int flags, ...) {
	if (should_fake(pathname)) {
		return open_fake_fd(pathname);
	}

	orig_open_t orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
	return orig_open(pathname, flags);
}

int creat(const char *pathname, mode_t mode) {
	orig_creat_t orig_creat = (orig_creat_t)dlsym(RTLD_NEXT, "creat");
	return orig_creat(pathname, mode);
}

ssize_t read(int fd, void *buf, size_t count) {
	orig_read_t orig_read = (orig_read_t)dlsym(RTLD_NEXT, "read");
	return orig_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
	orig_write_t orig_write = (orig_write_t)dlsym(RTLD_NEXT, "write");
	return orig_write(fd, buf, count);
}

int unlink(const char *pathname) {
	orig_unlink_t orig_unlink = (orig_unlink_t)dlsym(RTLD_NEXT, "unlink");
	return orig_unlink(pathname);
}

int close(int fd) {
	if (is_fake_fd(fd)) {
		return close_fake_fd(fd);
	}

	orig_close_t orig_close = (orig_close_t)dlsym(RTLD_NEXT, "close");
	return orig_close(fd);
}

/* FILE functions */
typedef FILE *(*orig_fopen_t)(const char *path, const char *mode);
typedef FILE *(*orig_freopen_t)(const char *path, const char *mode, FILE *stream);
typedef int (*orig_fclose_t)(FILE *fp);

FILE *fopen(const char *path, const char *mode) {
	if (should_fake(path)) {
		return open_fake_file(path);
	}

	orig_fopen_t orig_fopen = (orig_fopen_t)dlsym(RTLD_NEXT, "fopen");
	return orig_fopen(path, mode);
}

int fclose(FILE *fp) {
	if (is_fake_file(fp)) {
		return close_fake_file(fp);
	}

	orig_fclose_t orig_fclose = (orig_fclose_t)dlsym(RTLD_NEXT, "fclose");
	return orig_fclose(fp);
}
