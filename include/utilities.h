#ifndef UTILITIES_H
#define UTILITIES_H

/*
 *	utilities.h
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

/* from utilities.c */

char *create_filebuffer(const char *);
void destroy_filebuffer(char *);
char **create_URL_buffer(const char *, unsigned *);
void destroy_URL_buffer(char **, unsigned);
int URL_is_valid(const char *);
unsigned uintlen(unsigned);
void animate_progress_bar(size_t);

#endif
