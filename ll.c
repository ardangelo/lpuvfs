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
	} entry;
	struct _node_t *next;
} node_t;

node_t *head = NULL;
node_t *tail = NULL;

/* list manipulation */

node_t *new_node() {
    node_t *n = (node_t*)malloc(sizeof(node_t));
    n->next = NULL;

    if (head == NULL) {
        head = n;
    } else {
        tail->next = n;
    }

    tail = n;

    return n;
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

/* file implementations */

fake_file_t* new_ff() {
	return (fake_file_t*)new_node();
}

fake_file_t* lookup_name(const char* name) {
    for (node_t *n = head; n != NULL; n = n->next) {
        if (!strncmp(n->entry.f.name, name, MAX_FN)) {
            return (fake_file_t*)n;
        }
    }

    return NULL;
}

fake_file_t* lookup_fd(int fd) {
    for (node_t *n = head; n != NULL; n = n->next) {
        if (n->entry.f.fd == fd) {
            return (fake_file_t*)n;
        }
    }

    return NULL;
}

fake_file_t* lookup_fp(const FILE* fp) {
    for (node_t *n = head; n != NULL; n = n->next) {
        if (n->entry.f.fp == fp) {
            return (fake_file_t*)n;
        }
    }

    return NULL;
}

void unlink_ff(fake_file_t *ff) {
	delete_node((node_t*)ff);
}

/* directory implementations */

fake_dir_t* lookup_dirp(const DIR* dirp) {
	for (node_t *n = head; n != NULL; n = n->next) {
        if ((DIR*)n == dirp) {
            return (fake_dir_t*)n;
        }
    }

    return NULL;
}

void unlink_dirp(DIR* dirp) {
	delete_node((node_t*)dirp);
}
