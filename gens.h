#ifndef GENS_H_
#define GENS_H_

int should_fake_file(const char *pathname);
char* generate_file_contents(const char *pathname);

int should_fake_dir(const char *pathname);
char** generate_dir_contents(const char *pathname);

#endif