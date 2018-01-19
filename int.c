#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* dynamic linking */
#include <dlfcn.h>

/* file control */
#include <fcntl.h>
#include <unistd.h>

/* directory control */
#include <sys/types.h>
#include <dirent.h>

#include "vfs.h"

/* file function types */
typedef int (*orig_open_t)(const char *pathname, int flags, ...);
typedef int (*orig_creat_t)(const char *pathname, mode_t mode);
typedef ssize_t (*orig_read_t)(int fd, void *buf, size_t count);
typedef ssize_t (*orig_write_t)(int fd, const void *buf, size_t count);
typedef int (*orig_unlink_t)(const char *pathname);
typedef int (*orig_close_t)(int fd);

/* directory function types */
typedef int (*orig_closedir_t)(DIR *dirp);
typedef DIR* (*orig_opendir_t)(const char *name);
typedef struct dirent* (*orig_readdir_t)(DIR *dirp);
typedef int (*orig_readdir_r_t)(DIR *dirp, struct dirent *entry, struct dirent **result);
typedef void (*orig_rewinddir_t)(DIR *dirp);
typedef void (*orig_seekdir_t)(DIR *dirp, long int loc);
typedef long int (*orig_telldir_t)(DIR *dirp);

/* file info types */
typedef int (*orig_stat_t)(const char *pathname, struct stat *statbuf);

/* fd functions */

int open(const char *pathname, int flags, ...) {
	if (should_fake_file(pathname)) {
		return open_fake_fd(pathname);
	}

	orig_open_t orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
	return orig_open(pathname, flags);
}
int open64(const char *pathname, int flags, ...) {
	return open(pathname, flags);
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
	if (should_fake_file(path)) {
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


/* directory functions */

int closedir(DIR *dirp) {
	fprintf(stderr, "caught closedir\n");

	orig_closedir_t orig_closedir = (orig_closedir_t)dlsym(RTLD_NEXT, "closedir");
	return orig_closedir(dirp);
}

DIR* opendir(const char *name) {
	if (should_fake_dir(name)) {
		
	}

	orig_opendir_t orig_opendir = (orig_opendir_t)dlsym(RTLD_NEXT, "opendir");
	return orig_opendir(name);
}

struct dirent* readdir(DIR *dirp) {
	fprintf(stderr, "caught readdir\n");

	orig_readdir_t orig_readdir = (orig_readdir_t)dlsym(RTLD_NEXT, "readdir");
	return orig_readdir(dirp);
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
	fprintf(stderr, "caught readdir_r\n");

	orig_readdir_r_t orig_readdir_r = (orig_readdir_r_t)dlsym(RTLD_NEXT, "readdir_r");
	return orig_readdir_r(dirp, entry, result);
}

void rewinddir(DIR *dirp) {
	fprintf(stderr, "caught rewinddir\n");

	orig_rewinddir_t orig_rewinddir = (orig_rewinddir_t)dlsym(RTLD_NEXT, "rewinddir");
	return orig_rewinddir(dirp);
}

void seekdir(DIR *dirp, long int loc) {
	fprintf(stderr, "caught seekdir\n");

	orig_seekdir_t orig_seekdir = (orig_seekdir_t)dlsym(RTLD_NEXT, "seekdir");
	return orig_seekdir(dirp, loc);
}

long int telldir(DIR *dirp) {
	fprintf(stderr, "caught telldir\n");

	orig_telldir_t orig_telldir = (orig_telldir_t)dlsym(RTLD_NEXT, "telldir");
	return orig_telldir(dirp);
}

/* file info functions */
int stat(const char *pathname, struct stat *statbuf) {
	fprintf(stderr, "caught stat\n");

	orig_stat_t orig_stat = (orig_stat_t)dlsym(RTLD_NEXT, "stat");
	return orig_stat(pathname, statbuf);
}
int stat64(const char *pathname, struct stat *statbuf) {
	return stat(pathname, statbuf);
}
