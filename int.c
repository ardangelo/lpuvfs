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
typedef int (*orig_lstat_t)(const char *pathname, struct stat *statbuf);

/* fd functions */

int open(const char *pathname, int flags, ...) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_file(fixed_path)) {
		return open_fake_fd(fixed_path);
	}

	static orig_open_t orig_open = NULL;
	if (orig_open == NULL) {
		orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
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

	static orig_close_t orig_close = NULL;
	if (orig_close == NULL) {
		orig_close = (orig_close_t)dlsym(RTLD_NEXT, "close");
	}
	return orig_close(fd);
}

/* FILE functions */

typedef FILE *(*orig_fopen_t)(const char *path, const char *mode);
typedef FILE *(*orig_freopen_t)(const char *path, const char *mode, FILE *stream);
typedef int (*orig_fclose_t)(FILE *fp);

FILE *fopen(const char *pathname, const char *mode) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_file(fixed_path)) {
		return open_fake_file(fixed_path);
	}

	static orig_fopen_t orig_fopen = NULL;
	if (orig_fopen == NULL) {
		orig_fopen = (orig_fopen_t)dlsym(RTLD_NEXT, "fopen");
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

	static orig_fclose_t orig_fclose = NULL;
	if (orig_fclose == NULL) {
		orig_fclose = (orig_fclose_t)dlsym(RTLD_NEXT, "fclose");
	}
	return orig_fclose(fp);
}


/* directory functions */

int closedir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return close_fake_dir(dirp);
	}

	static orig_closedir_t orig_closedir = NULL;
	if (orig_closedir == NULL) {
		orig_closedir = (orig_closedir_t)dlsym(RTLD_NEXT, "closedir");
	}
	return orig_closedir(dirp);
}

DIR* opendir(const char *pathname) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	if (should_fake_dir(fixed_path)) {
		return (DIR*)open_fake_dir(fixed_path);
	}

	static orig_opendir_t orig_opendir = NULL;
	if (orig_opendir == NULL) {
		orig_opendir = (orig_opendir_t)dlsym(RTLD_NEXT, "opendir");
	}
	return orig_opendir(pathname);
}

struct dirent* readdir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return read_fake_dir(dirp);
	}

	static orig_readdir_t orig_readdir = NULL;
	if (orig_readdir == NULL) {
		orig_readdir = (orig_readdir_t)dlsym(RTLD_NEXT, "readdir");
	}
	return orig_readdir(dirp);
}
struct dirent* readdir64(DIR *dirp) {
	return readdir(dirp);
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	static orig_readdir_r_t orig_readdir_r = NULL;
	if (orig_readdir_r == NULL) {
		orig_readdir_r = (orig_readdir_r_t)dlsym(RTLD_NEXT, "readdir_r");
	}
	return orig_readdir_r(dirp, entry, result);
}

void rewinddir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	if (is_fake_dirp(dirp)) {
		return rewind_fake_dir(dirp);
	}

	static orig_rewinddir_t orig_rewinddir = NULL;
	if (orig_rewinddir == NULL) {
		orig_rewinddir = (orig_rewinddir_t)dlsym(RTLD_NEXT, "rewinddir");
	}
	return orig_rewinddir(dirp);
}

void seekdir(DIR *dirp, long int loc) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	static orig_seekdir_t orig_seekdir = NULL;
	if (orig_seekdir == NULL) {
		orig_seekdir = (orig_seekdir_t)dlsym(RTLD_NEXT, "seekdir");
	}
	return orig_seekdir(dirp, loc);
}

long int telldir(DIR *dirp) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);

	static orig_telldir_t orig_telldir = NULL;
	if (orig_telldir == NULL) {
		orig_telldir = (orig_telldir_t)dlsym(RTLD_NEXT, "telldir");
	}
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

	static orig_stat_t orig_stat = NULL;
	if (orig_stat == NULL) {
		orig_stat = (orig_stat_t)dlsym(RTLD_NEXT, "stat");
	}
	return orig_stat(pathname, statbuf);
}
int stat64(const char *pathname, struct stat *statbuf) {
	return stat(pathname, statbuf);
}

int lstat(const char *pathname, struct stat *statbuf) {
	if (DEBUG) fprintf(stderr, "caught %s\n", __func__);
	static char fixed_path[MAX_PATH];
	canonicalize(pathname, fixed_path);

	record_t rec = create_fake_record(fixed_path);
	if (rec.type != NO_REC) {
		if (DEBUG) fprintf(stderr, "record for %s\n", pathname);
		return fill_statbuf(statbuf, rec);
	}

	if (DEBUG) fprintf(stderr, "no record for %s\n", pathname);
	static orig_lstat_t orig_lstat = NULL;
	if (orig_lstat == NULL) {
		orig_lstat = (orig_lstat_t)dlsym(RTLD_NEXT, "lstat");
	}
	return orig_lstat(pathname, statbuf);
}
int lstat64(const char *pathname, struct stat *statbuf) {
	return lstat(pathname, statbuf);
}