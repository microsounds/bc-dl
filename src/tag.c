#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "membuf.h"
#include "parse.h"
#include "tag.h"
#include "utilities.h"

/*
 *	tag.c
 *	This file is part of bc-dl.
 *	See bc-dl.c for copyright or LICENSE for license information.
 */

/* struct initializer order must match enum definition order by design */

const frame_t ID3_FRAME[NUMBER_OF_FRAMES] = {
	{ "TIT2", TIT2 },
	{ "TPE1", TPE1 },
	{ "TALB", TALB },
	{ "TDRC", TDRC },
	{ "TRCK", TRCK },
	{ "COMM", COMM },
	{ "TPE2", TPE2 },
	{ "APIC", APIC }
};

/* ID3v2.4 FRAME / ENUM TABLE
 * TIT2 | Song Title
 * TPE1 | Artist
 * TALB | Album Title
 * TDRC | Year
 * TRCK | Track No.
 * COMM | Comment
 * TPE2 | Album Artist
 * APIC | Embedded Image
 */

/* ID3v2.4 LAYOUT
 * +-----------------+
 * | ID3v2.4 header  |
 * +---+-------------+---+
 *     | Frame header    |
 *     +---+-------------+---+
 *         | Frame 'XXXX'    |
 *     +---+-------------+---+
 *     | Frame header    |
 *     +---+-------------+---+
 *         | Frame 'XXXX'    |
 * +-------+---------+-------+
 * | ID3v2.4 footer  | (OPTIONAL)
 * +-----------------+
 * | MP3 SYNCWORD    |
 * +-----------------+
 */

void *memmem(void *memblk, size_t h_len, void *matchblk, size_t n_len)
{
	char *haystack = (char *) memblk;
	char *needle = (char *) matchblk;
	if (n_len > h_len)
		goto end;

	unsigned i, j;
	for (i = 0; i < h_len; i++)
	{
		if (i + n_len < h_len) /* bounds checking */
		{
			unsigned matches = 0;
			for (j = 0; j < n_len; j++)
			{
				if (haystack[i + j] == needle[j])
					matches++;
			}
			if (matches == n_len)
				return (void *) haystack + i;
		}
		else
			goto end;
	}
	end: return NULL;
}

int is_ASCII(const char *str)
{
	/* detects if string is 7-bit ASCII */
	unsigned len = strlen(str);
	unsigned char *data = (unsigned char *) str;
	unsigned i;
	for (i = 0; i < len; i++)
	{
		if (data[i] > 0x7F) /* 127 */
			return 0;
	}
	return 1;
}

void id3_text_field(char *frame, size_t *offset, char *text)
{
	/* Text encoding    $xx
	 * Information    <text string according to encoding>
	 */
	char encoding; /* detect encoding */
	if (is_ASCII(text))
		encoding = 0x00; /* ISO-8859-1 */
	else
		encoding = 0x03; /* UTF-8 */
	memcpy(frame + *offset, &encoding, 1);
	*offset += 1;

	/* copy null terminated string */
	memcpy(frame + *offset, text, strlen(text) + 1);
	*offset += strlen(text) + 1;
}

void id3_comment_field(char *frame, size_t *offset, char *comment)
{
	/* Text encoding           $xx
	 * Language                $xx xx xx
	 * Short content descrip.  <text string according to encoding> $00 (00)
	 * The actual text         <full text string according to encoding>
	 */
	char padding[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
	memcpy(frame + *offset, padding, 5);
	*offset += 5;
	memcpy(frame + *offset, comment, strlen(comment) + 1);
	*offset += strlen(comment) + 1;
}

void id3_embedded_image(char *frame, size_t *offset, membuf_t *art)
{
	/* Text encoding   $xx
	 * MIME type       <text string> $00
	 * Picture type    $xx
	 * Description     <text string according to encoding> $00 (00)
	 * Picture data    <binary data>
	 */
	char encoding[] = { 0x00 }; /* ISO-8859-1 */
	memcpy(frame + *offset, encoding, 1);
	*offset += 1;
	char mime_type[] = "image/jpeg"; /* MIME type */
	memcpy(frame + *offset, mime_type, 11);
	*offset += 11;
	char pic_type[] = { 0x03 }; /* Cover (front) */
	memcpy(frame + *offset, pic_type, 1);
	*offset += 1;
	char desc[] = { 0x00 }; /* empty desc */
	memcpy(frame + *offset, desc, 1);
	*offset += 1;
	char *picture_data = art->memory; /* binary data */
	size_t data_size = art->size;
	memcpy(frame + *offset, picture_data, data_size);
	*offset += data_size;
}

void id3_track_numbering(char *frame, size_t *offset, int track, int track_count)
{
	/* create string from track number, pass it off as normal text field */
	unsigned tr_len = uintlen(track+1);
	unsigned tr_count_len = uintlen(track);
	if (track+1 < 10) /* 01, 02, 03... */
		tr_len++;
	if (track_count < 10)
		tr_count_len++;

	char *tr_str = (char *) malloc(sizeof(char) * tr_len + 2);
	if (track+1 < 10) /* track number */
		sprintf(tr_str, "0%u", track+1);
	else
		sprintf(tr_str, "%u", track+1);

	char *tr_count_str = (char *) malloc(sizeof(char) * tr_count_len + 2);
	if (track_count < 10) /* total tracks */
		sprintf(tr_count_str, "0%u", track_count);
	else
		sprintf(tr_count_str, "%u", track_count);

	char *numbering = (char *) malloc(sizeof(char) * tr_len + tr_count_len + 3);
	sprintf(numbering, "%s/%s", tr_str, tr_count_str); /* 04/09 */
	id3_text_field(frame, offset, numbering);
	free(tr_str);
	free(tr_count_str);
	free(numbering);
}

size_t id3_read_28bit_length(void *start)
{
	/* converts 4-byte 28-bit uint value to unsigned long long
	 * length value is decoded from a 28-bit uint value across 4 bytes
	 * using only 7 effective bits per byte, MSB (bit 7) is skipped for each byte
	 * ID3v2 size              4 * %0xxxxxxx
	 */
	char *location = (char *) start;
	const unsigned BIT_LENGTH = 7;
	const unsigned WORD_SIZE = 4;
	size_t length = 0;
	int i, j;
	unsigned k = 0; /* length integer bit index */
	for (i = WORD_SIZE - 1; i >= 0; i--)
	{
		for (j = 0; j < BIT_LENGTH; j++)
		{
			if (location[i] & (1 << j))
				length |= (1 << k);
			k++;
		}
	}
	return length + ID3_HEADER_LENGTH;
}

void id3_write_28bit_length(size_t length, void *start)
{
	/* converts unsigned long long to 4 byte 28-bit uint value
	 * length value is encoded to a 28-bit uint value across 4 bytes
	 * using only 7 effective bits per byte, MSB (bit 7) is skipped for each byte
	 * ID3v2 size              4 * %0xxxxxxx
	 */
	char *location = (char *) start;
	const unsigned BIT_LENGTH = 7;
	const unsigned WORD_SIZE = 4;
	int i, j;
	unsigned k = 0; /* length integer bit index */
	for (i = WORD_SIZE - 1; i >= 0; i--)
	{
		for (j = 0; j < BIT_LENGTH; j++)
		{
			if (length & (1 << k++))
				location[i] |= (1 << j);
		}
	}
}

char *id3_create_frame(const frame_t *id3, size_t *len, unsigned track, album_t *album, membuf_t *art)
{
	const unsigned BIG_ENOUGH = art->size * 2;
	char *frame = (char *) calloc(sizeof(char), BIG_ENOUGH);
	/* Frame ID       $xx xx xx xx (four characters)
	 * Size           $xx xx xx xx
	 * Flags          $xx xx
	 */
	size_t offset = 0; /* total size */

	/* write frame header */
	memcpy(frame+offset, id3->id, strlen(id3->id));
	offset += strlen(id3->id);
	char padding[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	memcpy(frame+offset, padding, 6);
	offset += 6;

	/* write frame data */
	switch (id3->frame)
	{
		case TIT2: id3_text_field(frame, &offset, album->song_titles[track]); break;
		case TPE1: id3_text_field(frame, &offset, album->artist); break;
		case TALB: id3_text_field(frame, &offset, album->album_title); break;
		case TDRC: id3_text_field(frame, &offset, album->release_date); break;
		case TPE2: id3_text_field(frame, &offset, album->album_artist); break;
		case TRCK: id3_track_numbering(frame, &offset, track, album->track_count); break;
		case COMM: id3_comment_field(frame, &offset, album->comment); break;
		case APIC: id3_embedded_image(frame, &offset, art); break;
		default: break;
	}

	/* write total size of frame to frame header */
	id3_write_28bit_length(offset - ID3_HEADER_LENGTH, frame + ID3_FRAME_LEN_OFFSET);
	frame = (char *) realloc(frame, offset); /* shrink */
	*len = offset; /* pass total length to external scope */
	return frame;
}

membuf_t *write_id3_tags(membuf_t *file, membuf_t *art, album_t *album, unsigned track)
{
	/* write ID3v2.4 tags to a new membuf
	 * if tags exist in file, truncate tags
	 * concat new tag + file membufs
	 * return concatenated membuf
	 */
	fflush(stdout);

	/* ID3v2/file identifier   "ID3"
	 * ID3v2 version           $03 00
	 * ID3v2 flags             %abc00000
	 * ID3v2 size              4 * %0xxxxxxx
	 */
	char v3_header_seq[] = { 0x49, 0x44, 0x33, 0x03, 0x00, 0x00 }; /* ID3v2.3.0 */
	char v4_header_seq[] = { 0x49, 0x44, 0x33, 0x04, 0x00, 0x00 }; /* ID3v2.4.0 */
	char size_padding[] = { 0x00, 0x00, 0x00, 0x00 };

	/* search for first 4 bytes of ID3 header */
	char *tag = (char *) memmem(file->memory, file->size, v3_header_seq, strlen(v3_header_seq));
	if (tag != NULL) /* if tag exists, truncate tags from file */
	{
		size_t tag_length = id3_read_28bit_length(tag + ID3_HEADER_LEN_OFFSET);
		file->size -= tag_length;
		memmove(file->memory, file->memory+tag_length, file->size);
		file->memory = (char *) realloc(file->memory, file->size);
	}

	/* create tags */
	membuf_t *id3_tag = membuf_init();
	id3_tag->filename = NULL; /* this won't be used */
	membuf_write(v4_header_seq, 6, 1, id3_tag);
	membuf_write(size_padding, 4, 1, id3_tag); /* 4 bytes padding for 28-bit tag size */
	unsigned i;
	for (i = 0; i < NUMBER_OF_FRAMES; i++) /* generate ID3 frames */
	{
		size_t frame_length = 0; /* includes frame headers */
		char *frame = id3_create_frame(&ID3_FRAME[i], &frame_length, track, album, art);
		membuf_write(frame, frame_length, 1, id3_tag);
		free(frame);
	}
	/* write total size of tag to tag header */
	id3_write_28bit_length(id3_tag->size - ID3_HEADER_LENGTH, id3_tag->memory + ID3_HEADER_LEN_OFFSET);

	/* concatenate tag + file */
	char *merged = (char *) malloc(sizeof(char) * id3_tag->size + file->size);
	memcpy(merged, id3_tag->memory, id3_tag->size);
	memcpy(&merged[id3_tag->size], file->memory, file->size);
	file->size += id3_tag->size;
	membuf_free(id3_tag);
	free(file->memory);
	file->memory = merged;
	return file;
}
