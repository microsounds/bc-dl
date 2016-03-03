#ifndef TAG_H
#define TAG_H

/*
 *	tag.h
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

/* from tag.c */

/* CONSTANTS */

#define ID3_HEADER_LENGTH 10
#define ID3_HEADER_LEN_OFFSET 6
#define ID3_FRAME_LEN_OFFSET 4

/* ID3 FRAMES DEFINED HERE */

#define NUMBER_OF_FRAMES 8

enum _id3_frame_type {
	TIT2, TPE1, TALB, TDRC,
	TRCK, COMM, TPE2, APIC
};

struct _id3_frame {
	char *id;
	enum _id3_frame_type frame;
};

typedef struct _id3_frame frame_t;

membuf_t *write_id3_tags(membuf_t *, membuf_t *, album_t *, unsigned);

#endif
