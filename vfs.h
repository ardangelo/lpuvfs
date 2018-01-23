#ifndef VFS_H_
#define VFS_H_

#include "lpuvfs.h"

/* file functions */

int is_fake_fd(int fd);
int open_fake_fd(const char *pathname);
int close_fake_fd(int fd);

int is_fake_file(FILE *fp);
FILE *open_fake_file(const char *path);
int close_fake_file(FILE *fp);

/* dir functions */

int is_fake_dirp(DIR *dirp);

DIR *open_fake_dir(const char *pathname);
struct dirent *read_fake_dir(DIR *dirp);
struct dirent64 *read_fake_dir64(DIR *dirp);
void rewind_fake_dir(DIR *dirp);
int close_fake_dir(DIR *dirp);

/* stat functions */

int should_fake_file(const char *pathname);
int should_fake_dir(const char *pathname);
record_t create_fake_record(const char *pathname);
int fill_statbuf(struct stat *statbuf, record_t rec);
int fill_statbuf64(struct stat64 *statbuf, record_t rec);
char* canonicalize(const char *pathname, char *buf);

#endif
