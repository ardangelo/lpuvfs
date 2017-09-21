#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* memory management */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* file control */
#include <fcntl.h>

#include "vfs.h"

#define MAX_FN 255

typedef struct _fake_file_t {
	char *buf;
	size_t size;
	int fd;
	FILE *fp;
	const char *name;
} fake_file_t;

/* management functions */

typedef struct _node_t {
    fake_file_t ff;
    struct _node_t *next;
} node_t;

node_t *head = NULL;
node_t *tail = NULL;

fake_file_t* new_ff() {
    node_t *n = (node_t*)malloc(sizeof(node_t));
    n->next = NULL;

    if (head == NULL) {
        head = n;
    } else {
        tail->next = n;
    }

    tail = n;

    return &n->ff;
}


fake_file_t* lookup_name(const char* name) {
    node_t* n = head;

    for (node_t *n = head; n != NULL; n = n->next) {
        if (!strncmp(n->ff.name, name, MAX_FN)) {
            return &n->ff;
        }
    }

    return NULL;
}

fake_file_t* lookup_fd(int fd) {
    for (node_t *n = head; n != NULL; n = n->next) {
        if (n->ff.fd == fd) {
            return &n->ff;
        }
    }

    return NULL;
}

fake_file_t* lookup_fp(const FILE* fp) {
    for (node_t *n = head; n != NULL; n = n->next) {
        if (n->ff.fp == fp) {
            return &n->ff;
        }
    }

    return NULL;
}

void delete_node(node_t* target) {
    if (head == NULL) {
        return;
    }

    node_t *prev = head;
    for (node_t *n = head->next; n != NULL; n = n->next) {
        if (n->next == target) {
            if (prev != NULL) {
                prev->next = n->next;
            }
            free(n);

            return;
        }
    }

    return;
}

const char str[] = "tricked you!";

int should_fake(const char *pathname) {
	return (!strncmp(pathname, "/fake/", 6));
}

/* fd functions */

int is_fake_fd(int fd) {
	return lookup_fd(fd) != NULL;
}

int open_fake_fd(const char *pathname) {
	fake_file_t *ff = lookup_name(pathname);
	if (ff == NULL) {
	    ff = new_ff();
	} else {
	    return ff->fd;
	}

	ff->size = getpagesize();
	ff->name = pathname;
	ff->fd = shm_open(ff->name, O_CREAT | O_RDWR, 0777);

	if (ff->fd > 0) {
		ftruncate(ff->fd, ff->size);
		ff->buf = (char*)mmap(0, ff->size, PROT_READ | PROT_WRITE, MAP_SHARED, ff->fd, 0);
		sprintf(ff->buf, "fd %d: %s\n", ff->fd, str);
	}

	return ff->fd;
}

int close_fake_fd(int fd) {
    fake_file_t *ff = lookup_fd(fd);
	if (ff == NULL) {
	    return -1;
	}

	munmap(ff->buf, ff->size);
	shm_unlink(ff->name);
	free(ff);
	ff = NULL;
	return 0;
}

/* FILE* functions */

int is_fake_file(FILE *fp) {
	return lookup_fp(fp) != NULL;
}

FILE *open_fake_file(const char *pathname) {
    fake_file_t *ff = lookup_name(pathname);
	if (ff != NULL) {
	    return ff->fp;
	}

	int fd = open_fake_fd(pathname);
	FILE *fp = fdopen(fd, "w+");
	fprintf(fp, "FILE* from fd %d: %s\n", fd, str);
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
