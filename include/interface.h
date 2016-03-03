#ifndef INTERFACE_H
#define INTERFACE_H

/*
 *	interface.h
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

/* from interface.c */

enum _filename_mode {
	FILE_MODE = 0,
	FOLDER_MODE = 1
};

void download_album_at_URL(const char *);

#endif
