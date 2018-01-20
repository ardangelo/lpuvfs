#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* memory management */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* file control */
#include <fcntl.h>

#include "ll.h"
#include "vfs.h"

/* implementation eventually delegated to the client program */

int should_fake_file(const char *pathname) {
	return (!strncmp(pathname, "/fake/", 6)) && (pathname[strlen(pathname) - 1] != '/');
}

int should_fake_dir(const char *pathname) {
	return (!strncmp(pathname, "/fake/", 6)) && (pathname[strlen(pathname) - 1] == '/');
}

char* generate_file_contents(const char *pathname) {
	// no asprintf in this QNX :(
	const char fmt_str_1[] = "tricked you! `";
	const char fmt_str_2[] = "` doesn't exist!";
	char *full = malloc(strlen(fmt_str_1) + strlen(fmt_str_2) + strlen(pathname) + 1);
	strcpy(full, fmt_str_1);
	strcpy(full + strlen(fmt_str_1), pathname);
	strcpy(full + strlen(fmt_str_1) + strlen(pathname), fmt_str_2);

	return full;
}

char** generate_folder_contents(const char *pathname) {
	char **res = (char**)malloc(3 * sizeof(char*));
	res[0] = strdup("file1");
	res[1] = strdup("file2");
	res[2] = NULL;
	return res;
}

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

	ff->size = getpagesize();
	ff->pathname = pathname;
	ff->fd = shm_open(ff->pathname, O_CREAT | O_RDWR, 0777);

	if (ff->fd > 0) {
		ftruncate(ff->fd, ff->size);
		ff->buf = (char*)mmap(0, ff->size, PROT_READ | PROT_WRITE, MAP_SHARED, ff->fd, 0);
		char *contents = generate_file_contents(pathname);
		sprintf(ff->buf, "fd %d: %s\n", ff->fd, contents);
		free(contents);
	}

	return ff->fd;
}

int close_fake_fd(int fd) {
	fake_file_t *ff = lookup_fd(fd);
	if (ff == NULL) {
		return -1;
	}

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
	char* contents = generate_file_contents(pathname);
	fprintf(fp, "FILE* from fd %d: %s\n", fd, contents);
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
		char **contents = generate_folder_contents(pathname);
		size_t numents = 3; // ., .., NULL terminator
		for (char **p = contents; *p != NULL; p++, numents++);
		fd->dirents = (struct dirent **)malloc(numents * sizeof(struct dirent *));
		for (size_t i = 0; i < numents - 1; i++) {
			const char *entname = NULL;
			if (i == 0) {
				entname = ".";
			} else if (i == 1) {
				entname = "..";
			} else {
				entname = contents[i - 2];
			}
			size_t namelen = strnlen(entname, MAX_FN);
			fd->dirents[i] = (struct dirent *)malloc(sizeof(struct dirent) + namelen + 1);
			fd->dirents[i]->d_ino = 1;
			fd->dirents[i]->d_namelen = namelen;
			strncpy(fd->dirents[i]->d_name, entname, namelen);
			free(contents[i]);
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

void fill_statbuf(struct stat *statbuf, int is_dir) {
	mode_t ro_mode = S_IXUSR | S_IRUSR | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH;

	statbuf->st_ino = 1;
	statbuf->st_size = 4;
	statbuf->st_dev = 666;
	statbuf->st_rdev = 666;
	statbuf->st_uid = 1001;
	statbuf->st_gid = 1001;
	statbuf->st_mtime = 0; 
	statbuf->st_atime = 0;
	statbuf->st_ctime = 0;
	if (is_dir) {
		statbuf->st_mode = ro_mode | S_IFDIR;
	} else {
		statbuf->st_mode = ro_mode | S_IFREG;
	}
	statbuf->st_nlink = 0;
	statbuf->st_blocksize = 4;
	statbuf->st_nblocks = 1;
	statbuf->st_blksize = 4;
	statbuf->st_blocks = 1;
}