#ifndef VFS_H_
#define VFS_H_

/* file functions */

int should_fake_file(const char *pathname);

int is_fake_fd(int fd);
int open_fake_fd(const char *pathname);
int close_fake_fd(int fd);

int is_fake_file(FILE *fp);
FILE *open_fake_file(const char *path);
int close_fake_file(FILE *fp);

/* dir functions */

int should_fake_dir(const char *pathname);

int is_fake_dirp(DIR *dirp);

DIR *open_fake_dir(const char *pathname);
struct dirent *read_fake_dir(DIR *dirp);
void rewind_fake_dir(DIR *dirp);
int close_fake_dir(DIR *dirp);

/* stat functions */

void fill_statbuf(struct stat *statbuf, int is_dir);

#endif
