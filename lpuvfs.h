#ifndef LPUVFS_H_
#define LPUVFS_H_

int register_file_generator(const char *pathname, char* (*file_gen)(const char*));
int register_dir_generator(const char *pathname, char** (*dir_gen)(const char*));

#endif