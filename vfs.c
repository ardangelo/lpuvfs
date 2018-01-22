#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* memory management */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* file control */
#include <libgen.h>
#include <fcntl.h>

#include "ll.h"
#include "vfs.h"

/* fd functions */

int is_fake_fd(int fd) {
	return lookup_fd(fd) != NULL;
}

int open_fake_fd(const char *pathname) {
	fake_file_t *ff = lookup_filename(pathname);
	if (ff == NULL) {
		ff = new_ff();
	} else {
		return ff->fd;
	}

	ff->pathname = pathname;
	ff->fd = shm_open(ff->pathname, O_CREAT | O_RDWR, 0777);

	if (ff->fd > 0) {
		fs_gen_t *fsg = lookup_fs_gen(pathname);
		if (fsg == NULL) {
			return -1;
		}
		char *contents = fsg->file_gen(pathname);
		ff->size = strlen(contents);

		ftruncate(ff->fd, ff->size);
		ff->buf = (char*)mmap(0, ff->size, PROT_READ | PROT_WRITE, MAP_SHARED, ff->fd, 0);
		sprintf(ff->buf, "%s\n", contents);
		free(contents);
	}

	return ff->fd;
}

int close_fake_fd(int fd) {
	fake_file_t *ff = lookup_fd(fd);
	if (ff == NULL) {
		return -1;
	}

	/* clean up the memory mappings */
	munmap(ff->buf, ff->size);
	shm_unlink(ff->pathname);
	unlink_ff(ff);
	ff = NULL;
	return 0;
}

/* FILE* functions */

int is_fake_file(FILE *fp) {
	return lookup_fp(fp) != NULL;
}

FILE* open_fake_file(const char *pathname) {
	fake_file_t *ff = lookup_filename(pathname);
	if (ff != NULL) {
		return ff->fp;
	}

	int fd = open_fake_fd(pathname);
	FILE *fp = fdopen(fd, "w+");
	fs_gen_t *fsg = lookup_fs_gen(pathname);
	if (fsg == NULL) {
		return NULL;
	}
	char *contents = fsg->file_gen(pathname);
	fprintf(fp, "%s\n", contents);
	free(contents);
	rewind(fp);

	return fp;
}

int close_fake_file(FILE *fp) {
	fake_file_t *ff = lookup_fp(fp);
	if (ff == NULL) {
		return -1;
	}

	fclose(ff->fp);
	free(ff->buf);
	close_fake_fd(ff->fd);
}

/* directory functions */

int is_fake_dirp(DIR *dirp) {
	return lookup_dirp(dirp) != NULL;
}

DIR* open_fake_dir(const char *pathname) {
	fake_dir_t *fd = lookup_dirname(pathname);
	if (fd == NULL) {
		fd = new_fd();

		// fill in the dirents
		fs_gen_t *fsg = lookup_fs_gen(pathname);
		if (fsg == NULL) {
			return NULL;
		}
		record_t *contents = fsg->dir_gen(pathname);
		size_t numents = 3; // ., .., NULL terminator
		for (record_t *p = contents; p->name != NULL; p++, numents++);
		fd->dirents = (struct dirent **)malloc(numents * sizeof(struct dirent *));
		for (size_t i = 0; i < numents - 1; i++) {
			const char *entname = NULL;
			if (i == 0) {
				entname = ".";
			} else if (i == 1) {
				entname = "..";
			} else {
				entname = contents[i - 2].name;
			}
			size_t namelen = strnlen(entname, MAX_FN);
			fd->dirents[i] = (struct dirent *)malloc(sizeof(struct dirent) + namelen + 1);
			fd->dirents[i]->d_ino = 1;
			fd->dirents[i]->d_namelen = namelen;
			strncpy(fd->dirents[i]->d_name, entname, namelen);
		}
		fd->dirents[numents] = NULL;
		fd->cur = &fd->dirents[0];
		free(contents);
	}

	return (DIR*)fd;
}

struct dirent* read_fake_dir(DIR *dirp) {
	fake_dir_t *fd = (fake_dir_t*)dirp;
	struct dirent *d = *(fd->cur);
	if (d != NULL) {
		(fd->cur)++;
	}

	return d;
}

void rewind_fake_dir(DIR *dirp) {
	fake_dir_t *fd = (fake_dir_t*)dirp;
	fd->cur = &fd->dirents[0];
}

int close_fake_dir(DIR *dirp) {
	fake_dir_t *fd = (fake_dir_t*)dirp;
	for (struct dirent **p = fd->dirents; *p != NULL; p++) {
		free(*p);
	}
	free(fd->dirents);
	free(fd);

	return 0;
}


/* stat functions */

int register_fs_gen(const char *pathname, char* (*file_gen)(const char*), record_t* (*dir_gen)(const char*), rec_type_t (*type_gen)(const char*)) {
	fs_gen_t *fsg = new_fs_gen();
	fsg->pathname = strdup(pathname);
	fsg->file_gen = file_gen;
	fsg->dir_gen = dir_gen;
	fsg->type_gen = type_gen;

	return 0;
}

int should_fake_file(const char *pathname) {
	fs_gen_t *fsg = lookup_fs_gen(pathname);
	return (fsg != NULL) && (fsg->type_gen(pathname) == FILE_REC);
}

int should_fake_dir(const char *pathname) {
	fs_gen_t *fsg = lookup_fs_gen(pathname);
	return (fsg != NULL) && (fsg->type_gen(pathname) == DIR_REC);
}

record_t create_fake_record(const char *pathname) {
	record_t res;
	fs_gen_t *fsg = lookup_fs_gen(pathname);
	if (fsg == NULL) {
		res.type = NO_REC;
		return res;
	}

	res.name = basename(pathname);
	res.size = 4;
	res.type = fsg->type_gen(pathname);

	return res;
}

int fill_statbuf(struct stat *statbuf, record_t rec) {
	mode_t ro_mode = S_IXUSR | S_IRUSR | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH;

	statbuf->st_ino = 1;
	statbuf->st_size = rec.size;
	statbuf->st_dev = 666;
	statbuf->st_rdev = 666;
	statbuf->st_uid = 1001;
	statbuf->st_gid = 1001;
	statbuf->st_mtime = 0; 
	statbuf->st_atime = 0;
	statbuf->st_ctime = 0;
	if (rec.type == DIR_REC) {
		statbuf->st_mode = ro_mode | S_IFDIR;
	} else {
		statbuf->st_mode = ro_mode | S_IFREG;
	}
	statbuf->st_nlink = 0;
	statbuf->st_blocksize = 4;
	statbuf->st_nblocks = 1;
	statbuf->st_blksize = 4;
	statbuf->st_blocks = rec.size / statbuf->st_blksize;

	return 0;
}

char* canonicalize(const char *pathname, char *buf) {
	if (buf == NULL) {
		buf = strdup(pathname);
	} else {
		strcpy(buf, pathname);
	}
	// TODO: remove trailing slash, dots, etc

	return buf;
}