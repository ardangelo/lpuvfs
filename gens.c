#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ll.h"

#define DEBUG 0

/* registry */

int register_file_generator(const char *pathname, char* (*file_gen)(const char*)) {
	if (DEBUG) fprintf(stderr, "registering file gen for %s\n", pathname);
	file_gen_t *fg = new_file_gen();
	fg->pathname = strdup(pathname);
	fg->gen = file_gen;

	return 0;
}

int register_dir_generator(const char *pathname, char** (*dir_gen)(const char*)) {
	if (DEBUG) fprintf(stderr, "registering dir gen for %s\n", pathname);
	dir_gen_t *dg = new_dir_gen();
	dg->pathname = strdup(pathname);
	dg->gen = dir_gen;

	return 0;
}

/* search */

int should_fake_file(const char *pathname) {
	if (DEBUG) fprintf(stderr, "searching file gen for %s\n", pathname);
	return (pathname[strlen(pathname) - 1] != '/') && (lookup_file_gen(pathname) != NULL);
}

int should_fake_dir(const char *pathname) {
	if (DEBUG) fprintf(stderr, "searching dir gen for %s\n", pathname);
	return (pathname[strlen(pathname) - 1] == '/') && (lookup_dir_gen(pathname) != NULL);
}

/* generation */

char* generate_file_contents(const char *pathname) {
	if (DEBUG) fprintf(stderr, "calling file gen for %s\n", pathname);
	file_gen_t *fg = lookup_file_gen(pathname);
	assert(fg != NULL);
	return fg->gen(pathname);
}

char** generate_dir_contents(const char *pathname) {
	if (DEBUG) fprintf(stderr, "calling dir gen for %s\n", pathname);
	dir_gen_t *dg = lookup_dir_gen(pathname);
	assert(dg != NULL);
	return dg->gen(pathname);
}