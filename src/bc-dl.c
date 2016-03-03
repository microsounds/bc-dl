#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "cli.h"
#include "interface.h"
#include "utilities.h"

/*
 *	bc-dl - basic CLI downloader for bandcamp.com
 *	Copyright (C) 2016 microsounds
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Open an issue on Github if you encounter any problems.
 * https://github.com/microsounds/bc-dl
 *
 * For building, installation and usage information, see README.md.
 */

/* GLOBAL ABOUT */

struct _global GLOBAL = {
	.title = "bc-dl",
	.desc = "basic CLI downloader for bandcamp.com",
	.ver = "0.5.0",
	.author = "microsounds",
	.year = "2016",
	.license = "GNU General Public License v3.0"
};

/* MAIN */

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 3) /* invalid usage */
	{
		program_identification(NORMAL);
		program_usage(VERBOSE);
		return 1;
	}
	fmode_t mode = get_mode(argv[1]);
	if (mode == MODE_HELP) /* -h, --help */
	{
		program_usage(NORMAL);
		program_help();
		goto end;
	}
	else if (mode == MODE_VERSION) /* -v, --version */
	{
		program_identification(VERBOSE);
		goto end;
	}
	else if (mode == MODE_MULTI) /* -i, --iterate */
	{
		program_identification(NORMAL);
		if (argv[2])
		{
			char *buf = create_filebuffer(argv[2]);
			unsigned elements = 0;
			char **url_buffer = create_URL_buffer(buf, &elements);
			destroy_filebuffer(buf);
			unsigned i;
			for (i = 0; i < elements; i++)
			{
				progress_indicator("Job", i+1, elements, url_buffer[i]);
				if (URL_is_valid(url_buffer[i]))
					download_album_at_URL(url_buffer[i]);
				else
					program_error(ERROR_INVALID_URL);
			}
			destroy_URL_buffer(url_buffer, elements);
			goto end;
		}
		else
		{
			program_usage(VERBOSE);
			return 1;
		}
	}
	else /* MODE_NORMAL */
	{
		if (strlen(argv[1]) < 6) /* still invalid */
		{
			program_identification(NORMAL);
			program_usage(VERBOSE);
			return 1;
		}
		program_identification(NORMAL);
		progress_indicator("Job", 1, 1, argv[1]);
		if (URL_is_valid(argv[1]))
			download_album_at_URL(argv[1]);
		else
			program_error(ERROR_INVALID_URL);
	}

	end: return 0;
}
