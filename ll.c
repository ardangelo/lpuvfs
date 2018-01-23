#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* file control */
#include <fcntl.h>

#include "ll.h"

typedef struct _node_t {
	union _entry {
		fake_file_t f;
		fake_dir_t d;
		fs_gen_t fsg;
	} entry;
	struct _node_t *next;
} node_t;

typedef struct _list_t {
	node_t *h;
	node_t *t;
} list_t;

list_t *file_list = NULL;
list_t *dir_list = NULL;
list_t *fs_gen_list = NULL;

/* list manipulation */

node_t *new_node(list_t **lp) {
	list_t *l = *lp;
	if (l == NULL) {
		l = (list_t*)malloc(sizeof(list_t));
		*lp = l;
		l->h = NULL;
		l->t = NULL;
	}

	node_t *n = (node_t*)malloc(sizeof(node_t));
	n->next = NULL;

	if (l->h == NULL) {
		l->h = n;
	} else {
		l->t->next = n;
	}

	l->t = n;

	return n;
}

void delete_node(list_t *l, node_t* target) {
	if ((l == NULL) || (l->h == NULL)) {
		return;
	}

	node_t *prev = l->h;
	for (node_t *n = l->h->next; n != NULL; n = n->next) {
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

node_t *find_node(list_t *l, const char *pathname) {
	if (l == NULL) {
		return NULL;
	}

	for (node_t *n = l->h; n != NULL; n = n->next) {
		if (!strncmp((char*)n, pathname, MAX_FN)) {
			return n;
		}
	}

	return NULL;
}

/* file implementations */

fake_file_t* new_ff() {
	return (fake_file_t*)new_node(&file_list);
}

fake_file_t* lookup_filename(const char* name) {
	node_t *n = find_node(file_list, name);
	if (n != NULL) {
		return (fake_file_t*)n;
	}

	return NULL;
}

fake_file_t* lookup_fd(int fd) {
	if (file_list == NULL) {
		return NULL;
	}

	for (node_t *n = file_list->h; n != NULL; n = n->next) {
		if (n->entry.f.fd == fd) {
			return (fake_file_t*)n;
		}
	}

	return NULL;
}

fake_file_t* lookup_fp(const FILE *fp) {
	if (file_list == NULL) {
		return NULL;
	}

	for (node_t *n = file_list->h; n != NULL; n = n->next) {
		if (n->entry.f.fp == fp) {
			return (fake_file_t*)n;
		}
	}

	return NULL;
}

void unlink_ff(fake_file_t *ff) {
	delete_node(file_list, (node_t*)ff);
}

/* directory implementations */

fake_dir_t* new_fd() {
	return (fake_dir_t*)new_node(&dir_list);
}

fake_dir_t* lookup_dirname(const char* name) {
	node_t *n = find_node(dir_list, name);
	if (n != NULL) {
		return (fake_dir_t*)n;
	}

	return NULL;
}


fake_dir_t* lookup_dirp(const DIR *dirp) {
	fprintf(stderr, "lookup dirp %p\n", dirp);
	if (dir_list == NULL) {
		return NULL;
	}

	for (node_t *n = dir_list->h; n != NULL; n = n->next) {
		if ((DIR*)n == dirp) {
			return (fake_dir_t*)n;
		}
	}

	return NULL;
}

void unlink_fd(fake_dir_t *fd) {
	delete_node(dir_list, (node_t*)fd);
}

/* generators */

fs_gen_t* new_fs_gen() {
	return (fs_gen_t*)new_node(&fs_gen_list);
}

fs_gen_t* lookup_fs_gen(const char *pathname) {
	if (fs_gen_list == NULL) {
		return NULL;
	}

	for (node_t *n = fs_gen_list->h; n != NULL; n = n->next) {
		const char *pre = n->entry.fsg.pathname;
		if (!strncmp(pre, pathname, strlen(pre))) {
			return &n->entry.fsg;
		}
	}
}
