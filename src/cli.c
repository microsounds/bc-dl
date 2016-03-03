#include <stdio.h>
#include <string.h>

#include "global.h"
#include "cli.h"

/*
 *	cli.c
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

/* struct initializer order must match enum definition order by design */

/* PROGRAM ERROR HANDLING */

const struct _error ERROR_INDEX[NUMBER_OF_ERRORS] = {
	{.err = ERROR_INVALID_URL, .desc = "URL is invalid." },
	{.err = ERROR_CONNECTION, .desc = "Connection error." },
	{.err = ERROR_JSON, .desc = "JSON inconsistency error. Webpage layout might have changed. If this persists, contact maintainer." },
	{.err = ERROR_FILE_IO, .desc = "Cannot write to disk." },
	{.err = ERROR_MEM_IO, .desc = "Cannot expand memory buffer. Out of memory." }
};

void program_error(enum _error_flag err)
{
	fprintf(stderr, "[!] Error! -- ");
	fprintf(stderr, "%s\n", ERROR_INDEX[err].desc);
}

/* CLI OPTION FLAG INFORMATION */

const struct _cli_flags MODE_FLAGS[NUMBER_OF_MODES] = {
	{.flag = "-h", .gnuflag = "--help", .desc = "Display this help screen.", .mode = MODE_HELP },
	{.flag = "-v", .gnuflag = "--version", .desc = "Version and license information.", .mode = MODE_VERSION },
	{.flag = "-i", .gnuflag = "--iterate", .desc = "Provide iterated list of urls.", .mode = MODE_MULTI }
};


/* COMMAND LINE ROUTINES DEFINED HERE */

enum _flag_mode get_mode(const char *str)
{
	unsigned i;
	for (i = 0; i < NUMBER_OF_MODES; i++)
	{
		if (!strcmp(str, MODE_FLAGS[i].flag) ||
		    !strcmp(str, MODE_FLAGS[i].gnuflag))
			return MODE_FLAGS[i].mode;
	}
	return MODE_NORMAL;
}

void program_help(void)
{
	unsigned i;
	printf("help:\n");
	for (i = 0; i < NUMBER_OF_MODES; i++)
		printf("\t%s (%s) - %s\n",
		       MODE_FLAGS[i].flag,
		       MODE_FLAGS[i].gnuflag,
		       MODE_FLAGS[i].desc);
}

void program_identification(enum _verbose setting)
{
	printf("%s %s\n", GLOBAL.title, GLOBAL.ver);
	printf("%s\n", GLOBAL.desc);
	if (setting == VERBOSE)
	{
		printf("(c) %s %s\n", GLOBAL.year, GLOBAL.author);
		printf("Released under the %s\n", GLOBAL.license);
	}
}

void program_usage(enum _verbose setting)
{
	const char *flags = "[-h | -v | -i list.txt]";
	const char *example = "http://artist.bandcamp.com/album/example";
	const char *more = "Run with -h or --help for all options.";
	if (setting == VERBOSE)
		printf("%s\n", more);
	printf("usage:\n\t%s %s %s\n", GLOBAL.title, flags, example);
}

void progress_indicator(char *subject, unsigned current, unsigned total, char *comment)
{
	printf("%s %u of %u -- Downloading: '%s'\n", subject, current, total, comment);
}
