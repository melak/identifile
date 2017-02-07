/*
 * Copyright (c) 2009-2017 Tamas Tevesz <ice@extreme.hu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <magic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef HAVE_GETPAGESIZE
#define HAVE_GETPAGESIZE 1
#endif

extern char *__progname;
static __attribute__((noreturn)) void usage(void);

static __attribute__((noreturn)) void usage(void) {
	fprintf(stderr, "Usage: %s [-d] file\n", __progname);
	fprintf(stderr, "       -d   print the (not helpful) `data' type\n");
	exit(1);
}

int main(int argc, char **argv) {
	magic_t m;
	int ch, fd, suppress_data, ret, bufsize;
	off_t flen, offset;
	const char *magic_type;
	char *p;
	struct stat st;

	fd = -1;			/* silence */
	flen = 0;
	p = NULL;
	suppress_data = 1;		/* default */
	ret = 1;

	while ((ch = getopt(argc, argv, "dm")) != -1) {
		switch(ch) {
			case 'd':
				suppress_data = 0;
				break;
			default:
				usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	bufsize = (int)sysconf(_SC_PAGESIZE);
	if (bufsize == -1)
#if HAVE_GETPAGESIZE
		bufsize = getpagesize();
#else
		bufsize = 4096;
#endif

	m = magic_open(MAGIC_NONE);
	if (m == NULL) {
		warn("magic_open");
		goto out;
	}

	if (magic_load(m, NULL) == -1) {
		warnx("magic_load: %s", magic_error(m));
		goto out;
	}

	fd = open(argv[0], O_RDONLY|O_NONBLOCK);
	if (fd == -1) {
		warn("%s", optarg);
		goto out;
	}

	if (fstat(fd, &st) == -1) {
		warn("%s", optarg);
		goto out;
	}

	flen = st.st_size;

	p = (char *)mmap(NULL, (unsigned)flen, PROT_READ, MAP_PRIVATE, fd, 0);
	if (p == MAP_FAILED) {
		warn("mmap");
		goto out;
	}
#if !defined( MADV_NOCORE )
#	if defined( MADV_DONTDUMP )
#		define MADV_NOCORE MADV_DONTDUMP
#	else
#		define MADV_NOCORE 0
#	endif
#endif
	madvise(p, flen, MADV_WILLNEED | MADV_NOCORE);

	ret = 0;
	offset = -1;

	while (++offset < flen) {
		magic_type = magic_buffer(m, p + offset, flen - offset > bufsize ? bufsize : flen - offset);
		if (suppress_data && strncmp(magic_type, "data", 5) == 0)
			continue;
		printf("offset = %lld type = %s\n", (long long)offset, magic_type);
	}

out:
	if (p)
		munmap(p, flen);
	if (fd > -1)
		close(fd);
	if (m)
		magic_close(m);

	return ret;
}

