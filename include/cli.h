#ifndef CLI_H
#define CLI_H

/*
 *	cli.h
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */


/* from cli.c */

/* ERRORS DEFINED HERE */

#define NUMBER_OF_ERRORS 5

enum _error_flag {
	EVERYTHING_IS_FINE = -1, /* not a real error */
	ERROR_INVALID_URL,
	ERROR_CONNECTION,
	ERROR_JSON,
	ERROR_FILE_IO,
	ERROR_MEM_IO
};

struct _error {
	const enum _error_flag err;
	const char *desc;
};

/* CLI OPTION FLAGS DEFINED HERE */

#define NUMBER_OF_MODES 3

enum _flag_mode {
	MODE_NORMAL = -1, /* doesn't count as a real mode */
	MODE_HELP = 0,
	MODE_VERSION = 1,
	MODE_MULTI = 2
};

struct _cli_flags {
	const char *flag;
	const char *gnuflag;
	const char *desc;
	const enum _flag_mode mode;
};

enum _verbose {
	NORMAL,
	VERBOSE
};

void program_error(enum _error_flag);
enum _flag_mode get_mode(const char *);
void program_help(void);
void program_identification(enum _verbose);
void program_usage(enum _verbose);
void progress_indicator(char *, unsigned, unsigned, char *);

typedef enum _flag_mode fmode_t;
typedef enum _error_flag ferror_t;

#endif
