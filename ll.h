#ifndef LL_H_
#define LL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* file control */
#include <fcntl.h>

#include "lpuvfs.h"

typedef struct _fake_file_t {
	const char *pathname;
	char *buf;
	size_t size;
	int fd;
	FILE *fp;
} fake_file_t;

fake_file_t* new_ff();
void unlink_ff(fake_file_t *ff);

fake_file_t* lookup_filename(const char *name);
fake_file_t* lookup_fd(int fd);
fake_file_t* lookup_fp(const FILE *fp);

/* directory control */
#include <dirent.h>

typedef struct _fake_dir_t {
	const char *pathname;
	struct dirent **dirents;
	struct dirent **cur;
} fake_dir_t;

fake_dir_t* new_fd();
void unlink_fd(fake_dir_t *dirp);

fake_dir_t* lookup_dirname(const char *pathname);
fake_dir_t* lookup_dirp(const DIR *dirp);

/* generators */

typedef struct _fs_gen_t {
	const char *pathname;
	char* (*file_gen)(const char*);
	record_t* (*dir_gen)(const char*);
	rec_type_t (*type_gen)(const char*);
} fs_gen_t;

fs_gen_t* new_fs_gen();

fs_gen_t* lookup_fs_gen(const char *pathname);

#endif
