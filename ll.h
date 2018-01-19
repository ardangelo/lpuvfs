#ifndef LL_H_
#define LL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* file control */
#include <fcntl.h>

#define MAX_FN 255

typedef struct _fake_file_t {
	char *buf;
	size_t size;
	int fd;
	FILE *fp;
	const char *name;
} fake_file_t;

fake_file_t* new_ff();
void unlink_ff(fake_file_t *ff);

fake_file_t* lookup_name(const char* name);
fake_file_t* lookup_fd(int fd);
fake_file_t* lookup_fp(const FILE* fp);

/* directory control */
#include <dirent.h>

typedef struct _fake_dir_t {
	struct dirent *dirents;
	struct dirent *cur;
} fake_dir_t;

fake_dir_t* lookup_dirp(const DIR* dirp);
void unlink_dirp(DIR* dirp);

#endif
