#ifndef VFS_H_
#define VFS_H_

int should_fake(const char *pathname);

int is_fake_fd(int fd);
int open_fake_fd(const char *pathname);
int close_fake_fd(int fd);

int is_fake_file(FILE *fp);
FILE *open_fake_file(const char *path);
int close_fake_file(FILE *fp);

#endif
