#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h> /* libcurl */

#include "membuf.h"
#include "cli.h"
#include "utilities.h"

/*
 *	membuf.c
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

size_t membuf_write(void *ptr, size_t size, size_t nmemb, void *stream)
{
	/* simulate fwrite(), write to memory instead */
	size_t realsize = size * nmemb;
	membuf_t *mem = (membuf_t *) stream;
	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL)
	{
		program_error(ERROR_MEM_IO);
		abort();
	}
	memcpy(&mem->memory[mem->size], ptr, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	static unsigned progress = 0;
	if (progress++ == 5) /* flush stdout only sparingly */
	{
		animate_progress_bar(mem->size); /* progress bar */
		progress = 0;
	}
	return realsize;
}

membuf_t *membuf_init(void)
{
	membuf_t *out = (membuf_t *) malloc(sizeof(membuf_t));
	out->memory = (char *) malloc(sizeof(char));
	out->size = 0;
	return out;
}

void membuf_free(membuf_t *ptr)
{
	ptr->size = 0;
	free(ptr->memory);
	if (ptr->filename)
		free(ptr->filename);
	free(ptr);
}

membuf_t *membuf_download(const char *url, char *filename)
{
	fflush(stdout);
	CURL *dl_context = curl_easy_init();
	membuf_t *membuf = membuf_init();
	membuf->filename = filename;
	if (dl_context)
	{
		curl_easy_setopt(dl_context, CURLOPT_URL, url);
 		curl_easy_setopt(dl_context, CURLOPT_WRITEFUNCTION, membuf_write);
		curl_easy_setopt(dl_context, CURLOPT_WRITEDATA, (void *) membuf);
		curl_easy_setopt(dl_context, CURLOPT_FOLLOWLOCATION, 1); /* redirects */
		CURLcode res = curl_easy_perform(dl_context); /* download */
	 	curl_easy_cleanup(dl_context);
		if (res != CURLE_OK) /* download error */
 		{
 			program_error(ERROR_CONNECTION);
			abort();
 		}
 		else
 			return membuf;
	}
	else
		return NULL;
}

void membuf_commit_to_disk(membuf_t *ptr)
{
	animate_progress_bar(ptr->size);
	printf("Writing to: '%s'...", ptr->filename);
	fflush(stdout);
	FILE *file = fopen(ptr->filename, "w+");
	if (!file)
	{
		program_error(ERROR_FILE_IO);
		abort();
	}
	fwrite(ptr->memory, ptr->size, 1, file);
	fflush(file);
	fclose(file);
	printf("done.\n");
}
