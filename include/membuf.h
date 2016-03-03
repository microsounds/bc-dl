#ifndef MEMBUF_H
#define MEMBUF_H

/*
 *	membuf.h
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

/* from membuf.c */

struct _membuf {
	char *memory;
	size_t size;
	char *filename; /* optional */
};

typedef struct _membuf membuf_t;

size_t membuf_write(void *, size_t, size_t, void *);
membuf_t *membuf_init(void);
membuf_t *membuf_download(const char *, char *);
void membuf_commit_to_disk(membuf_t *);
void membuf_free(membuf_t *);

#endif
