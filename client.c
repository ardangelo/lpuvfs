#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lpuvfs.h"

#define DEBUG 1

rec_type_t generate_type(const char *pathname) {
	if (DEBUG) fprintf(stderr, "client: generate type for %s: ", pathname);
	if (!strcmp("/fake", pathname) || !strcmp("/fake/dir1", pathname) ||
		!strcmp("/fake/.", pathname) || !strcmp("/fake/dir1/.", pathname)) {
		if (DEBUG) fprintf(stderr, "directory\n");
		return DIR_REC;
	} else if (!strcmp("/fake/file1", pathname)) {
		if (DEBUG) fprintf(stderr, "file\n");
		return FILE_REC;
	}

	if (DEBUG) fprintf(stderr, "none\n");
	return NO_REC;
}

char* generate_file_contents(const char *pathname) {
	if (DEBUG) fprintf(stderr, "client: generate file contents for %s: ", pathname);
	const char fmt_str_1[] = "tricked you! `";
	const char fmt_str_2[] = "` doesn't exist!\n";
	char *full = malloc(strlen(fmt_str_1) + strlen(fmt_str_2) + strlen(pathname) + 1);
	strcpy(full, fmt_str_1);
	strcpy(full + strlen(fmt_str_1), pathname);
	strcpy(full + strlen(fmt_str_1) + strlen(pathname), fmt_str_2);

	return full;
}

record_t* generate_dir_contents(const char *pathname) {
	if (DEBUG) fprintf(stderr, "client: generate dir contents for %s: ", pathname);
	if (strcmp("/fake", pathname)) {
		record_t *res = (record_t*)malloc(sizeof(record_t));
		res[0].name = NULL;
		return res;
	}

	/* dir requested is root */
	record_t *res = (record_t*)malloc(3 * sizeof(record_t));
	res[0].name = strdup("file1");
	res[0].size = 4;
	res[0].type = FILE_REC;
	res[1].name = strdup("dir1");
	res[1].size = 4;
	res[1].type = DIR_REC;
	res[2].name = NULL;
	return res;
}

static void __attribute__((constructor)) _init() {
	fprintf(stderr, "loading client\n");
	register_fs_gen("/fake", &generate_file_contents, &generate_dir_contents, &generate_type);
}
