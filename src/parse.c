#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "membuf.h"
#include "parse.h"
#include "cli.h"

/*
 *	parse.c
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

char *create_substring(const char *haystack, char *start, char *end)
{
	/* determine range to copy by subracting ptrs between start and end strings
	 * copy diff-offset bytes from haystack to new substring
	 */
	char *from, *to;
	long diff;
	long offset;

	from = strstr(haystack, start);
	to = strstr(from, end);
	diff = to - from;
	offset = strlen(start);

	char *out = (char *) malloc(sizeof(char) * diff-offset + 1);
	strncpy(out, from+offset, diff-offset);
	out[diff-offset] = '\0'; /* null terminate */
	return out;
}

unsigned count_occurances_of(char *haystack, char *keyword)
{
	/* count the number of times keyword appears in the haystack */
	unsigned count = 0;
	const char *TRACK_INFO = "trackinfo : [{";
	char *start_of_data = strstr(haystack, TRACK_INFO);
	char *tmp = create_substring(start_of_data, "trackinfo", "var CurrencyData");
	/* operates on the assumption that 'var CurrencyData will
	   never be moved above the trackinfo block */
	char *tok = strtok(tmp, ":\",");
	while (tok != NULL)
	{
		if (!strcmp(tok, keyword))
			count++;
		tok = strtok(NULL, ":\",");
	}
	free(tmp);
	return count;
}

char **iterative_data_scraper(int iters, char *haystack, char *start, char *end)
{
	/* iterate over haystack, obtain data encapsulated between start + end strings
	   return 2D char array containing scraped data */
	char **char_array = (char **) malloc(sizeof(char *) * iters);
	const char *TRACK_INFO = "trackinfo : [{";
	char *start_of_data = strstr(haystack, TRACK_INFO);
	char *tmp = create_substring(start_of_data, "trackinfo", "var CurrencyData");
	char *tmp_start = tmp; /* so the handle doesn't disappear in the heap */
	/* operates on the assumption that 'var CurrencyData will
	   never be moved above the trackinfo block */
	unsigned i;
	for (i = 0; i < iters; i++)
	{
		char_array[i] = create_substring(tmp, start, end);
		tmp = strstr(tmp, char_array[i]); /* advance ptr */
		tmp += strlen(char_array[i]); /* skip forward */

		/* if string is really short, shorter than 9 bytes, just skip 600 bytes forward
		   scraper tends to grab the same string 2+ times if you don't do this */
		if (strlen(start) > strlen(char_array[i]))
			tmp += 600;
	}
	free(tmp_start);
	return char_array;
}

char **prefix_stream_urls(int iters, char **stream_urls)
{
	/* resize stream_url strings and prefix with insert */
	const char *insert = "http://";
	unsigned i;
	for (i = 0; i < iters; i++)
	{
		char *new_str = (char *) malloc(sizeof(char) * strlen(insert) + strlen(stream_urls[i]) + 1);
		sprintf(new_str, "%s%s", insert, stream_urls[i]);
		free(stream_urls[i]);
		stream_urls[i] = new_str;
	}
	return stream_urls;
}

int duplicates_urls_exist(album_t *data)
{
	/* check if any 2 url strings are identical */
	unsigned i, j;
	for (i = 0; i < data->track_count; i++)
	{
		for (j = i + 1; j < data->track_count; j++)
		{
			if (!strcmp(data->stream_urls[i], data->stream_urls[j]))
				return 1;
		}
	}
	return 0;
}

int power(int base, int exp)
{
	if (exp > 0)
	{
		int result = base;
		unsigned i;
		for (i = 0; i < exp - 1; i++)
			result *= base;
		return result;
	}
	else
		return 1;
}

int hex_to_dec(const char hex)
{
	if (hex >= '0' && hex <= '9')
		return hex - '0';
	else if (hex >= 'A' && hex <= 'F')
		return hex - 'A' + 10;
	else
		return 0;
}

char *rewrite_unicode_to_ascii(char *str)
{
	/* bandcamp sometimes leaves a stray '\u003C' unicode escape code
	 * where it shouldn't. escape codes for ASCII characters get passed
	 * through as text and make it to the ID3 tags.
	 * rewrites 7-bit unicode literals to ASCII
	 */
	const char unicode_literal[] = "\\u00";
	if (strstr(str, unicode_literal))
	{
		char *hit = strstr(str, unicode_literal);
		char val = 0; /* '\u003C' => '<' */
		unsigned i;
		unsigned exp = 3;
		for (i = 2; i < 6; i++)
		{
			/* hopefully, 16^3 hex digits never occur in the wild */
			val += hex_to_dec(hit[i]) * power(16, exp);
			exp--;
		}
		hit[0] = val;
		/* shift string 5 bytes left, overlapping bytes 1 thru 5
		 * hit +  0 1 2 3 4 5 6 7 8 9 A B C D E F
		 * before \ u 0 0 3 C / 3
		 * mid    < u 0 0 3 C / 3
		 * after  < / 3
		 */
		memmove(hit+1, hit+6, strlen(hit+6) + 1);
		str = (char *) realloc(str, strlen(str) + 1);
	}
	return str;
}


album_t *scrub_unicode_literals(album_t *data)
{
	/* fields most likely to have unicode literals */
	data->album_title = rewrite_unicode_to_ascii(data->album_title);
	data->artist = rewrite_unicode_to_ascii(data->artist);
	data->album_artist = rewrite_unicode_to_ascii(data->album_artist);
	unsigned i;
	for (i = 0; i < data->track_count; i++)
		data->song_titles[i] = rewrite_unicode_to_ascii(data->song_titles[i]);
	return data;
}

album_t *parse_album_data(membuf_t *ptr)
{
	/* parse JSON, scrape data into album container struct */
	const char *JSON_START = "var BandData";
	char *start_of_data = strstr(ptr->memory, JSON_START);
	if (!start_of_data)
	{
		program_error(ERROR_JSON);
		abort();
	}
	album_t *data = (album_t *) malloc(sizeof(album_t));

	/* URL album art */
	data->url_album_art = create_substring(start_of_data, "artFullsizeUrl: \"", "\",");

	/* album title */
	data->album_title = create_substring(start_of_data, "album_title: \"", "\",");

	/* artist */
	data->artist = create_substring(start_of_data, "artist: \"", "\",");

	/* album artist */
	data->album_artist = create_substring(start_of_data, "name: \"", "\",");

	/* comment */
	char *pt1 = "Visit ";
	char *pt2 = create_substring(start_of_data, "linkback: \"", "\" + \"");
	data->comment = (char *) malloc(sizeof(char) * strlen(pt1) + strlen(pt2) + 2);
	sprintf(data->comment, "%s%s/", pt1, pt2);
	free(pt2);

	/* release date */
	char *date_str = create_substring(start_of_data, "album_release_date: \"", "\",");
	char *tok = strtok(date_str, " ");
	unsigned d; /* just get the year */
	for (d = 0; d < 2; d++)
		tok = strtok(NULL, " ");
	if (strlen(tok) != 4) /* if this fails, everything else is probably broken too */
	{
		program_error(ERROR_JSON);
		abort();
	}
	else
	{
		char *date = (char *) malloc(sizeof(char) * strlen(tok) + 1);
		strcpy(date, tok);
		data->release_date = date;
		free(date_str); tok = NULL;
	}

	/* filetype */
	data->filetype = create_substring(start_of_data, "track?enc=", "-128&");

	/* track count */
	data->track_count = count_occurances_of(start_of_data, "track_id");

	/* song titles */
	data->song_titles = iterative_data_scraper(data->track_count, start_of_data, "\"title\":\"", "\",");

	/* stream urls */
	data->stream_urls = iterative_data_scraper(data->track_count, start_of_data, "\"mp3-128\":\"//", "\"},");
	data->stream_urls = prefix_stream_urls(data->track_count, data->stream_urls);

	/* sanitize certain fields for '\u003C' unicode literals */
	data = scrub_unicode_literals(data);

	/* just in case */
	if (duplicates_urls_exist(data))
	{
		program_error(ERROR_JSON);
		abort();
	}

	return data;
}

void display_album_data(album_t *ptr)
{
	printf("** Album Information\n");
	printf("%s - %s\n", ptr->artist, ptr->album_title);
	printf("Produced by %s.\n", ptr->album_artist);
	printf("Released %s, %s format.\n", ptr->release_date, ptr->filetype);
	printf("%u tracks.\n", ptr->track_count);
	unsigned i;
	for (i = 0; i < ptr->track_count; i++)
	{
		/* 01. Song Title */
		if (i+1 < 10)
			printf("0%u.", i+1);
		else
			printf("%u.", i+1);
		printf("%c", ' ');
		printf("%s\n", ptr->song_titles[i]);
	}
}

void free_album_data(album_t *ptr)
{
	free(ptr->url_album_art);
	free(ptr->release_date);
	free(ptr->comment);
	free(ptr->album_title);
	free(ptr->artist);
	free(ptr->album_artist);
	unsigned i;
	for (i = 0; i < ptr->track_count; i++)
	{
		free(ptr->song_titles[i]);
		free(ptr->stream_urls[i]);
	}
	free(ptr->song_titles);
	free(ptr->stream_urls);
	free(ptr->filetype);
	free(ptr);
}
