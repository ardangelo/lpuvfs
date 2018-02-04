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

#define DEBUG 0

/* file function types */
typedef int (*orig_open_t)(const char *pathname, int flags, ...);
typedef int (*orig_close_t)(int fd);

typedef FILE *(*orig_fopen_t)(const char *path, const char *mode);
typedef FILE *(*orig_fopen64_t)(const char *path, const char *mode);
typedef int (*orig_fclose_t)(FILE *fp);

/* directory function types */
typedef int (*orig_closedir_t)(DIR *dirp);
typedef DIR* (*orig_opendir_t)(const char *name);
typedef struct dirent* (*orig_readdir_t)(DIR *dirp);
typedef struct dirent64* (*orig_readdir64_t)(DIR *dirp);
typedef int (*orig_readdir_r_t)(DIR *dirp, struct dirent *entry, struct dirent **result);
typedef void (*orig_rewinddir_t)(DIR *dirp);
typedef void (*orig_seekdir_t)(DIR *dirp, long int loc);
typedef long int (*orig_telldir_t)(DIR *dirp);

/* file info types */
typedef int (*orig_stat_t)(const char *pathname, struct stat *statbuf);
typedef int (*orig_stat64_t)(const char *pathname, struct stat64 *statbuf);
typedef int (*orig_lstat_t)(const char *pathname, struct stat *statbuf);
typedef int (*orig_lstat64_t)(const char *pathname, struct stat64 *statbuf);

#define AUTOLOAD_ORIG(X) \
	static orig_##X##_t orig_##X = NULL; \
	if (orig_##X == NULL) { \
		orig_##X = (orig_##X##_t)dlsym(RTLD_NEXT, #X); \
	}

/* fd functions */

int open(const char *pathname, int flags, ...) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_file(fixed_path)) {
		return open_fake_fd(fixed_path);
	}

	AUTOLOAD_ORIG(open);
	return orig_open(pathname, flags);
}
int open64(const char *pathname, int flags, ...) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	return open(pathname, flags);
}

int close(int fd) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_fd(fd)) {
		return close_fake_fd(fd);
	}

	AUTOLOAD_ORIG(close);
	return orig_close(fd);
}

/* FILE functions */

FILE *fopen(const char *pathname, const char *mode) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_file(fixed_path)) {
		return open_fake_file(fixed_path);
	}

	AUTOLOAD_ORIG(fopen);
	return orig_fopen(pathname, mode);
}

FILE *fopen64(const char *pathname, const char *mode) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_file(fixed_path)) {
		return open_fake_file(fixed_path);
	}

	AUTOLOAD_ORIG(fopen64);
	return fopen64(pathname, mode);
}

int fclose(FILE *fp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_file(fp)) {
		return close_fake_file(fp);
	}

	AUTOLOAD_ORIG(fclose);
	return orig_fclose(fp);
}


/* directory functions */

int closedir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return close_fake_dir(dirp);
	}

	AUTOLOAD_ORIG(closedir);
	return orig_closedir(dirp);
}

DIR* opendir(const char *pathname) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_dir(fixed_path)) {
		return (DIR*)open_fake_dir(fixed_path);
	}

	AUTOLOAD_ORIG(opendir);
	return orig_opendir(pathname);
}

struct dirent* readdir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return read_fake_dir(dirp);
	}

	AUTOLOAD_ORIG(readdir);
	return orig_readdir(dirp);
}

struct dirent64* readdir64(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return read_fake_dir64(dirp);
	}

	AUTOLOAD_ORIG(readdir64);
	return orig_readdir64(dirp);
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	AUTOLOAD_ORIG(readdir_r);
	return orig_readdir_r(dirp, entry, result);
}

void rewinddir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return rewind_fake_dir(dirp);
	}

	AUTOLOAD_ORIG(rewinddir);
	return orig_rewinddir(dirp);
}

void seekdir(DIR *dirp, long int loc) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	AUTOLOAD_ORIG(seekdir);
	return orig_seekdir(dirp, loc);
}

long int telldir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	AUTOLOAD_ORIG(telldir);
	return orig_telldir(dirp);
}

/* file info functions */

int stat(const char *pathname, struct stat *statbuf) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	record_t rec = create_fake_record(fixed_path);
	if (rec.type != NO_REC) {
		return fill_statbuf(statbuf, rec);
	}

	AUTOLOAD_ORIG(stat);
	return orig_stat(pathname, statbuf);
}

int stat64(const char *pathname, struct stat64 *statbuf) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	record_t rec = create_fake_record(fixed_path);
	if (rec.type != NO_REC) {
		return fill_statbuf64(statbuf, rec);
	}

	AUTOLOAD_ORIG(stat64);
	return stat64(pathname, statbuf);
}

int lstat(const char *pathname, struct stat *statbuf) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	record_t rec = create_fake_record(fixed_path);
	if (rec.type != NO_REC) {
		return fill_statbuf(statbuf, rec);
	}

	AUTOLOAD_ORIG(lstat);
	return orig_lstat(pathname, statbuf);
}

int lstat64(const char *pathname, struct stat64 *statbuf) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	record_t rec = create_fake_record(fixed_path);
	if (rec.type != NO_REC) {
		return fill_statbuf64(statbuf, rec);
	}


	AUTOLOAD_ORIG(lstat64);
	return lstat64(pathname, statbuf);
}
