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

#define DEBUG 1

/* file function types */
typedef int (*orig_open_t)(const char *pathname, int flags, ...);
typedef int (*orig_close_t)(int fd);
typedef FILE *(*orig_fopen_t)(const char *path, const char *mode);
typedef FILE *(*orig_freopen_t)(const char *path, const char *mode, FILE *stream);
typedef int (*orig_fclose_t)(FILE *fp);

/* directory function types */
typedef int (*orig_closedir_t)(DIR *dirp);
typedef DIR* (*orig_opendir_t)(const char *name);
typedef struct dirent* (*orig_readdir_t)(DIR *dirp);
typedef int (*orig_readdir_r_t)(DIR *dirp, struct dirent *entry, struct dirent **result);
typedef void (*orig_rewinddir_t)(DIR *dirp);
typedef void (*orig_seekdir_t)(DIR *dirp, long int loc);
typedef long int (*orig_telldir_t)(DIR *dirp);
typedef int (*orig_chdir_t)(const char *path);

/* file info types */
typedef int (*orig_stat_t)(const char *pathname, struct stat *statbuf);
typedef int (*orig_lstat_t)(const char *pathname, struct stat *statbuf);

/* original function addresses */

orig_open_t orig_open = NULL;
orig_close_t orig_close = NULL;
orig_fopen_t orig_fopen = NULL;
orig_fclose_t orig_fclose = NULL;
orig_closedir_t orig_closedir = NULL;
orig_opendir_t orig_opendir = NULL;
orig_readdir_t orig_readdir = NULL;
orig_readdir_r_t orig_readdir_r = NULL;
orig_rewinddir_t orig_rewinddir = NULL;
orig_seekdir_t orig_seekdir = NULL;
orig_telldir_t orig_telldir = NULL;
orig_chdir_t orig_chdir = NULL;
orig_stat_t orig_stat = NULL;
orig_lstat_t orig_lstat = NULL;
int load_original_symbols() {
	orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
	orig_close = (orig_close_t)dlsym(RTLD_NEXT, "close");
	orig_fopen = (orig_fopen_t)dlsym(RTLD_NEXT, "fopen");
	orig_fclose = (orig_fclose_t)dlsym(RTLD_NEXT, "fclose");
	orig_closedir = (orig_closedir_t)dlsym(RTLD_NEXT, "closedir");
	orig_opendir = (orig_opendir_t)dlsym(RTLD_NEXT, "opendir");
	orig_readdir = (orig_readdir_t)dlsym(RTLD_NEXT, "readdir");
	orig_readdir_r = (orig_readdir_r_t)dlsym(RTLD_NEXT, "readdir_r");
	orig_rewinddir = (orig_rewinddir_t)dlsym(RTLD_NEXT, "rewinddir");
	orig_seekdir = (orig_seekdir_t)dlsym(RTLD_NEXT, "seekdir");
	orig_telldir = (orig_telldir_t)dlsym(RTLD_NEXT, "telldir");
	orig_chdir = (orig_chdir_t)dlsym(RTLD_NEXT, "chdir");
	orig_stat = (orig_stat_t)dlsym(RTLD_NEXT, "stat");
	orig_lstat = (orig_lstat_t)dlsym(RTLD_NEXT, "lstat");

	return 0;
}

/* fd functions */

int open(const char *pathname, int flags, ...) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_file(fixed_path)) {
		return open_fake_fd(fixed_path);
	}

	return orig_open(pathname, flags);
}
int open64(const char *pathname, int flags, ...) {
	return open(pathname, flags);
}

int close(int fd) {
	if (is_fake_fd(fd)) {
		return close_fake_fd(fd);
	}

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

	return orig_fopen(pathname, mode);
}
FILE *fopen64(const char *pathname, const char *mode) {
	return fopen(pathname, mode);
}

int fclose(FILE *fp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_file(fp)) {
		return close_fake_file(fp);
	}

	return orig_fclose(fp);
}


/* directory functions */

int closedir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return close_fake_dir(dirp);
	}

	return orig_closedir(dirp);
}

DIR* opendir(const char *pathname) {
	if (DEBUG) fprintf(stderr, "caught %s on %s\n", __func__, pathname);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_dir(fixed_path)) {
		return (DIR*)open_fake_dir(fixed_path);
	}

	return orig_opendir(pathname);
}

struct dirent* readdir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s on %p\n", __func__, dirp);

	void* p = NULL;
	fprintf(stderr, "%p\n", (void*)&p);

	if (is_fake_dirp(dirp)) {
		return read_fake_dir(dirp);
	}

	return orig_readdir(dirp);
}
struct dirent* readdir64(DIR *dirp) {
	return readdir(dirp);
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	return orig_readdir_r(dirp, entry, result);
}

void rewinddir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return rewind_fake_dir(dirp);
	}

	return orig_rewinddir(dirp);
}

void seekdir(DIR *dirp, long int loc) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	return orig_seekdir(dirp, loc);
}

long int telldir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	return orig_telldir(dirp);
}

int chdir(const char* path) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(path, fixed_path);

	if (should_fake_dir(fixed_path)) {
		// TODO: emulate changed directory by modifying canonicalize?
		return 0;
	}

	return orig_chdir(path);
}

/* file info functions */

int stat(const char *pathname, struct stat *statbuf) {
	if (DEBUG) fprintf(stderr, "caught %s on %s\n", __func__, pathname);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	record_t rec = create_fake_record(fixed_path);
	if (rec.type != NO_REC) {
		return fill_statbuf(statbuf, rec);
	}

	return orig_stat(pathname, statbuf);
}
int stat64(const char *pathname, struct stat *statbuf) {
	return stat(pathname, statbuf);
}

int lstat(const char *pathname, struct stat *statbuf) {
	if (DEBUG) fprintf(stderr, "caught %s on %s\n", __func__, pathname);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	record_t rec = create_fake_record(fixed_path);
	if (rec.type != NO_REC) {
		if (DEBUG) fprintf(stderr, "record for %s\n", fixed_path);
		return fill_statbuf(statbuf, rec);
	}

	if (DEBUG) fprintf(stderr, "no record for %s\n", fixed_path);

	return orig_lstat(pathname, statbuf);
}
int lstat64(const char *pathname, struct stat *statbuf) {
	return lstat(pathname, statbuf);
}