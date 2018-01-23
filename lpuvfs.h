#ifndef LPUVFS_H_
#define LPUVFS_H_

#define MAX_PATH 255
#define MAX_FN 48

typedef enum { NO_REC, FILE_REC, DIR_REC } rec_type_t;

typedef struct _record_t {
	const char *name;
	size_t size;
	rec_type_t type;
} record_t;

int load_original_symbols();
int register_fs_gen(const char *pathname,
                    char* (*file_gen)(const char*),
                    record_t* (*dir_gen)(const char*),
                    rec_type_t (*type_gen)(const char*));

#endif