#ifndef PARSE_H
#define PARSE_H

/*
 *	parse.h
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

/* from parse.c */

struct _album_container {
	char *url_album_art;
	char *release_date;
	char *comment;
	char *album_title;
	char *artist;
	char *album_artist;
	unsigned track_count;
	char **song_titles;
	char **stream_urls;
	char *filetype;
};

typedef struct _album_container album_t;

album_t *parse_album_data(membuf_t *);
void display_album_data(album_t *);
void free_album_data(album_t *);

#endif
