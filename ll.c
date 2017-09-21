#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* file control */
#include <fcntl.h>

#include "ll.h"

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
