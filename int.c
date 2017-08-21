/* gcc -shared -fPIC -g int.c -o int.so -ldl -lrt && gdb --args env LD_PRELOAD=$PWD/int.so ./a.out /fake/meme */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* file control */
#include <fcntl.h>

/* dynamic linking */
#include <dlfcn.h>

/* memory management */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef int (*orig_open_t)(const char *pathname, int flags, ...);
typedef int (*orig_creat_t)(const char *pathname, mode_t mode);
typedef ssize_t (*orig_read_t)(int fd, void *buf, size_t count);
typedef ssize_t (*orig_write_t)(int fd, const void *buf, size_t count);
typedef int (*orig_unlink_t)(const char *pathname);
typedef int (*orig_close_t)(int fd);

char *membuf = NULL;
int memsize = 0;
int memfd = -1;
const char *memname = "/faker";

const char str[] = "tricked you!\n";

int should_fake_fd(const char *pathname) {
	return (!strncmp(pathname, "/fake/", 6));
}

int is_fake_fd(int fd) {
	return (fd == -1);
}

int open_fake_fd(const char *pathname) {
	memsize = getpagesize();
	memfd = shm_open(memname, O_CREAT | O_RDWR, 0777);

	if (memfd > 0) {
		ftruncate(memfd, memsize);
		membuf = (char*)mmap(0, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
		strcpy(membuf, str);
	}

	return memfd;
}

int close_fake_fd(int fd) {
	munmap(membuf, memsize);
	shm_unlink(memname);
	return 0;
}

int open(const char *pathname, int flags, ...) {
	if (should_fake_fd(pathname)) {	
		return open_fake_fd(pathname);
	}
	
	orig_open_t orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
	return orig_open(pathname, flags);
}

int creat(const char *pathname, mode_t mode) {
	orig_creat_t orig_creat = (orig_creat_t)dlsym(RTLD_NEXT, "creat");
	return orig_creat(pathname, mode);
}

ssize_t read(int fd, void *buf, size_t count) {
	orig_read_t orig_read = (orig_read_t)dlsym(RTLD_NEXT, "read");
	return orig_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
	orig_write_t orig_write = (orig_write_t)dlsym(RTLD_NEXT, "write");
	return orig_write(fd, buf, count);
}

int unlink(const char *pathname) {
	orig_unlink_t orig_unlink = (orig_unlink_t)dlsym(RTLD_NEXT, "unlink");
	return orig_unlink(pathname);
}

int close(int fd) {
	if (is_fake_fd(fd)) {
		return close_fake_fd(fd);
	}
	
	orig_close_t orig_close = (orig_close_t)dlsym(RTLD_NEXT, "close");
	return orig_close(fd);
}

/* fread example: read an entire file */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main (int argc, char **argv) {
	int fd = open(argv[1], O_RDONLY);
	char buf[65];

	printf ("File handle %d\n", fd);
	
	int r = 0;
	while ((r = read(fd, buf, 64)) > 0) {
		buf[r] = '\0';
		printf(buf);
	}

	return 0;
}
