#ifndef LL_H_
#define LL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* file control */
#include <fcntl.h>

#define MAX_FN 48

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

typedef struct _file_gen_t {
	const char *pathname;
	char* (*gen)(const char*);
} file_gen_t;

file_gen_t* new_file_gen();
file_gen_t* lookup_file_gen(const char *pathname);

typedef struct _dir_gen_t {
	const char *pathname;
	char** (*gen)(const char*);
} dir_gen_t;

dir_gen_t* new_dir_gen();
dir_gen_t* lookup_dir_gen(const char *pathname);


#endif
