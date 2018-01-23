#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEL_CHAR '\1'
#define DEL_EOD '\2'

char* canonicalize(const char *pathname, char *buf) {
	if (buf == NULL) {
		buf = strdup(pathname);
	} else {
		strcpy(buf, pathname);
	}

	typedef enum { seen_trailing_slash, seen_dirname,
		seen_one_dot, seen_two_dots } state_t;
	state_t s = seen_trailing_slash;

	int up_level = 0;

	for (int i = strlen(buf); i >= 0; i--) {
		switch (s) {
			case seen_trailing_slash:
				switch (buf[i]) {
				case '\0':
					break;
				case '/':
					buf[i] = DEL_CHAR;
					break;
				case '.':
					s = seen_one_dot;
					break;
				default:
					s = seen_dirname;
					break;
				}
				break;
			case seen_dirname:
				switch (buf[i]) {
				case '/':
					s = seen_trailing_slash;
					if (up_level > 0) { 
						buf[i] = DEL_EOD;
						up_level--;
					}
					break;
				default:
					break;
				}
				break;
			case seen_one_dot:
				switch (buf[i]) {
				case '/':
					buf[i] = DEL_EOD;
					s = seen_trailing_slash;
					break;
				case '.':
					s = seen_two_dots;
					break;
				default:
					s = seen_dirname;
					break;
				}
				break;
			case seen_two_dots:
				switch (buf[i]) {
				case '/':
					buf[i] = DEL_EOD;
					s = seen_trailing_slash;
					up_level += 1;
					break;
				default:
					s = seen_dirname;
					break;
				}
				break;
		}
	}

	char *src = buf;
	char *dst = buf;
	while (*src != '\0') {
		if (*src == DEL_EOD) {
			*dst = '\0';
			do { src++; } while ((*src != '\0') && (*src != '/') && (*src != DEL_EOD));
		} else if (*src == DEL_CHAR) {
			*dst = '\0';
			src++;
		} else {
			*dst = *src;
			dst++;
			src++;
		}
	}
	*dst = '\0';
	if (buf[0] == '\0') {
		buf[0] = '/';
		buf[1] = '\0';
	}

	return buf;
}