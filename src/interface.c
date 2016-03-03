#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface.h"
#include "cli.h"
#include "utilities.h"
#include "membuf.h"
#include "parse.h"
#include "tag.h"

/*
 *	interface.c
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

int unsafe_character(char c)
{
	int ret = 0;
	switch (c)
	{
		/* strip /<>:|?*\'" and NULL bytes */
		case '/': ret = 1; break;
		case '<': ret = 1; break;
		case '>': ret = 1; break;
		case ':': ret = 1; break;
		case '|': ret = 1; break;
		case '?': ret = 1; break;
		case '*': ret = 1; break;
		case '\"': ret = 1; break;
		case '\\': ret = 1; break;
		case '\0': ret = 1; break;
		default: break;
	}
	return ret;
}

void sanitize_filename(char *filename, enum _filename_mode mode)
{
	/* replace all invalid filename chars with space */
	/* if FOLDER_MODE, check all but the last char
	   folders don't work when you take the / out */
	unsigned len = strlen(filename);
	unsigned i;
	if (mode == FOLDER_MODE)
		len -= 1;
	for (i = 0; i < len - 1; i++)
	{
		if (unsafe_character(filename[i]))
			filename[i] = ' ';
	}
}

char *create_folder_name(album_t *ptr)
{
 	#ifdef _WIN32
		char dir = '\\';
	#else
		char dir = '/';
	#endif

	/* malloc string large enough for 'Artist - Album (20XX)/'
	 * 8 bytes for formatting + null terminator
	 */
	char *str = (char *) malloc(sizeof(char) *
	                            strlen(ptr->artist) +
	                            strlen(ptr->album_title) +
	                            strlen(ptr->release_date) + 8);
	sprintf(str, "%s - %s (%s)%c", ptr->artist, ptr->album_title, ptr->release_date, dir);
	return str;
}

char *create_string(const char *str)
{
	/* create mutable string out of immutable string literal */
	char *out = (char *) malloc(sizeof(char) * strlen(str) + 1);
	sprintf(out, "%s", str);
	return out;
}

char *concat_strings(const char *str1, const char *str2)
{
	/* return concatenated string */
	char *concat = (char *) malloc(sizeof(char) * strlen(str1) + strlen(str2) + 1);
	sprintf(concat, "%s%s", str1, str2);
	return concat;
}

char *enquote_string(const char *str)
{
	/* surround string with "quotation marks" */
	char *enquote = (char *) malloc(sizeof(char) * strlen(str) + 3);
	sprintf(enquote, "\"%s\"", str);
	return enquote;
}

int create_folder(const char *folder)
{
	/* folder string needs to be quoted */
	char *quoted = enquote_string(folder);
	char *cmd = concat_strings("mkdir ", quoted);
	int err = system(cmd);
	free(cmd); free(quoted);
	if (!err)
		printf("Folder '%s' created.\n", folder);
	return err;
}

char *create_track_filename(album_t *album, unsigned track)
{
	/* calloc string large enough for '01. Song Title.mp3'
	 * 1 byte for leading zero if (track+1 > 10)
	 * 4 more for formatting + null terminator
	 */
	char *filename = (char *) calloc(uintlen(track+1) +
	                                 strlen(album->song_titles[track]) +
	                                 strlen(album->filetype) + 5,
	                                 sizeof(char));
	if (track+1 < 10)
		sprintf(filename, "0%u. %s.%s", track+1, album->song_titles[track], album->filetype);
	else
		sprintf(filename, "%u. %s.%s", track+1, album->song_titles[track], album->filetype);
	return filename;
}

int file_exists(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		long len = ftell(file);
		fclose(file);
		if (len > 0) /* if non-empty */
		{
			printf("Skipped: '%s', file exists.\n", filename);
			return len;
		}
	}
	return 0;
}

void download_album_at_URL(const char *url)
{
	/* files are cached in membuf before being written to disk
	 * filenames are stored with the membuf struct by design
	 */

	/* get album details */
	char *html_obj_name = create_string("album.html");
	membuf_t *html = membuf_download(url, html_obj_name);
	album_t *album = parse_album_data(html);
 	membuf_free(html);

	/* create folder */
	char *folder_name = create_folder_name(album);
	sanitize_filename(folder_name, FOLDER_MODE);
	create_folder(folder_name); /* collisions handled by shell */

	/* get cover art */
	char *art_filename = concat_strings(folder_name, "album.jpg");
	membuf_t *art = membuf_download(album->url_album_art, art_filename);
	if (!file_exists(art->filename))
		membuf_commit_to_disk(art);
	display_album_data(album);

	unsigned i;
	for (i = 0; i < album->track_count; i++)
	{
		char *filename = create_track_filename(album, i);
		sanitize_filename(filename, FILE_MODE);
		char *output_filename = concat_strings(folder_name, filename);
		progress_indicator("Track", i+1, album->track_count, filename);
		free(filename);
		if (!file_exists(output_filename))
		{
			membuf_t *track = membuf_download(album->stream_urls[i], output_filename);
			track = write_id3_tags(track, art, album, i);
			membuf_commit_to_disk(track);
			membuf_free(track);
		}
		else
			free(output_filename);
	}
	membuf_free(art);
	free(folder_name);
	free_album_data(album);
	printf("Completed.\n");
}
