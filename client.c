#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lpuvfs.h"

char* generate_file_contents(const char *pathname) {
	const char fmt_str_1[] = "tricked you! `";
	const char fmt_str_2[] = "` doesn't exist!";
	char *full = malloc(strlen(fmt_str_1) + strlen(fmt_str_2) + strlen(pathname) + 1);
	strcpy(full, fmt_str_1);
	strcpy(full + strlen(fmt_str_1), pathname);
	strcpy(full + strlen(fmt_str_1) + strlen(pathname), fmt_str_2);

	return full;
}

char** generate_dir_contents(const char *pathname) {
	char **res = (char**)malloc(3 * sizeof(char*));
	res[0] = strdup("file1");
	res[1] = strdup("file2");
	res[2] = NULL;
	return res;
}

static void init() __attribute__((constructor));

void init() {
	register_file_generator("/fake/", generate_file_contents);
	register_dir_generator("/fake/", generate_dir_contents);
}
