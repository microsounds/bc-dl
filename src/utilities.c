#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h> /* POSIX Regular Expressions */
#include <sys/ioctl.h> /* handles term width */
#include <unistd.h>

#include "utilities.h"

/*
 *  utilities.c
 *  This file is part of bc-dl.
 *  See bc-dl.c for copyright or LICENSE for license information.
 */

char *create_filebuffer(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		perror("[!] Could not open file.\n");
		abort();
	}
	fseek(fp, 0, SEEK_END);
	long pos = ftell(fp);
	char *buffer = (char *) malloc(sizeof(char) * pos + 1);
	fseek(fp, 0, SEEK_SET);
	fread(buffer, pos, 1, fp);
	fclose(fp);
	buffer[pos] = '\0'; /* null terminate */
	return buffer;
}

void destroy_filebuffer(char *buf)
{
	free(buf);
}

char **create_URL_buffer(const char *buf, unsigned *elements)
{
	unsigned max_len = 0;
	long buf_len = strlen(buf);
	char *tmp = (char *) malloc(sizeof(char) * buf_len + 1);
	strcpy(tmp, buf);
	char *tok = strtok(tmp, " \n");
	while (tok != NULL) /* dry run */
	{
		(*elements)++;
		unsigned current_len = strlen(tok);
		if (current_len > max_len)
			max_len = current_len;
		tok = strtok(NULL, " \n");
	}
	free(tmp);
	unsigned i; /* 2D array malloc */
	char **url_buffer = (char **) malloc(sizeof(char *) * *elements);
	for (i = 0; i < *elements; i++)
		url_buffer[i] = (char *) malloc(sizeof(char) * max_len + 1);
	char *tmp2 = (char *) malloc(sizeof(char) * buf_len + 1);
	strcpy(tmp2, buf);
	char *tok2 = strtok(tmp2, " \n");
	unsigned j = 0;
	while (tok2 != NULL) /* for real this time */
	{
		strcpy(url_buffer[j++], tok2);
		tok2 = strtok(NULL, " \n");
	}
	free(tmp2);
	return url_buffer;
}

void destroy_URL_buffer(char **buf, unsigned elements)
{
	unsigned i;
	for (i = 0; i < elements; i++)
		free(buf[i]);
	free(buf);
}

int URL_is_valid(const char *str)
{
	int reg_err;
	regex_t regex;
	char url_expr[] = "^(http|https)\\:\\/{2}.*\\.(bandcamp)\\..*\\/(album|single)\\/.*$";
	reg_err = regcomp(&regex, url_expr, REG_EXTENDED); /* compile */
	if (reg_err)
	{
		printf("%s\n", "INVALID REGEX");
		return 0;
	}
	reg_err = regexec(&regex, str, 0, NULL, 0);
	int valid = (reg_err != REG_NOMATCH);
	regfree(&regex);
	return valid;
}

unsigned uintlen(unsigned n)
{
	/* return number of places in a number */
	unsigned len = 1;
	while (n /= 10)
	{
		len++;
	}
	return len;
}

void clear_progress_bar(int width)
{
	printf("\r");
	unsigned i;
	for (i = 0; i < width; i++)
		printf("%c", ' ');
}

void human_readable_filesize(size_t filesize)
{
	static const unsigned long long bytes_in_KiB = 1024;
	static const unsigned long long bytes_in_MiB = 1048576;
	static const unsigned long long bytes_in_GiB = 1073741824;
	static const unsigned long long bytes_in_TiB = 1099511627776;
	static const unsigned long long bytes_in_PiB = 1125899906842624;
	static const unsigned long long bytes_in_EiB = 1152921504606846976;
/*	utilities.c:124:49: warning: integer constant is too large for its type
 *	static const unsigned long long bytes_in_ZiB = 1180591620717411303424ULL;
 *                                                 ^
 */
	if (filesize < bytes_in_KiB) /* B */
		printf("%zu B ", filesize);
	else if (filesize < bytes_in_MiB) /* KiB */
		printf("%.2lf KiB ", (double) filesize / bytes_in_KiB);
	else if (filesize < bytes_in_GiB) /* MiB */
		printf("%.2lf MiB ", (double) filesize / bytes_in_MiB);
	else if (filesize < bytes_in_TiB) /* GiB */
		printf("%.2lf GiB ", (double) filesize / bytes_in_GiB);
	else if (filesize < bytes_in_PiB) /* TiB */
		printf("%.2lf TiB ", (double) filesize / bytes_in_TiB);
	else if (filesize < bytes_in_EiB) /* PiB */
		printf("%.2lf PiB ", (double) filesize / bytes_in_PiB);
}

void animate_progress_bar(size_t data)
{
	/* animate progress bar one step per call */
	/* don't call it too often */
	/* LAG - number of steps before moving horizontally */
	/* HORI - horizonal length */
	/* ANI - number of animation frames */
	const char animation[] = { '/', '~', '\\', '|' };
	const unsigned LAG_MAX = 4;
	const unsigned HORI_MAX = 10;
	const unsigned ANI_MAX = 4;
	static unsigned lag_step = 0;
	static unsigned hori_step = 0;
	static unsigned ani_step = 0;

	fflush(stdout);

	struct winsize term;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &term);
	clear_progress_bar(term.ws_col); /* 80 */ /* prevent ghosting */

	printf("\r>>>> %s [", "Working");
	unsigned i, j;
	for (i = 0; i < hori_step; i++)
		printf("%c", ' ');
	printf("%c", animation[ani_step]);
	for (j = i + 1; j < HORI_MAX; j++)
		printf("%c", ' ');
	printf("] ");
	human_readable_filesize(data);

	lag_step++;
	ani_step++;

	if (lag_step == LAG_MAX)
	{
		lag_step = 0;
		hori_step++;
	}
	if (ani_step == ANI_MAX)
		ani_step = 0;
	if (hori_step == HORI_MAX)
		hori_step = 0;

	printf("\r");
	fflush(stdout);
}
