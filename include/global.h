#ifndef GLOBAL_H
#define GLOBAL_H

/*
 *	global.h
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

/* global decs  */

struct _global {
	const char *title;
	const char *desc;
	const char *ver;
	const char *author;
	const char *year;
	const char *license;
};

/* from bc-dl.c */

struct _global GLOBAL;

#endif
