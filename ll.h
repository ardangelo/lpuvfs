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

typedef struct _node_t {
    fake_file_t ff;
    struct _node_t *next;
} node_t;

fake_file_t* new_ff();

fake_file_t* lookup_name(const char* name);

fake_file_t* lookup_fd(int fd);

fake_file_t* lookup_fp(const FILE* fp);

#endif
