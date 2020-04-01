/*
 * ndc - numeric dump and conversion
 * Copyright (C) 2019-2020 Robert Imschweiler
 * 
 * This file is part of ndc.
 * 
 * ndc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ndc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ndc.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <limits.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DumpState.h"
#include "repository.h"
#include "util.h"
/* last */
#include "ndc.h"


/*
 * in                current chunk of input bytes
 * old               previous chunk of input bytes
 * out               current line of output
 * out_after_offset  pointer to end of offset representation in current output
 *                     line
 * out_after_dump    pointer to end of dump (= start of ascii_col) in current
 *                     output line
 * out_eol           pointer to current end of output line (after the last
 *                     character)
 * out_len           length of one output line
 * offset            current offset in input stream
 * processed         number of already processed bytes of current file
 * read_count        number of bytes that has been read to "in"
 * input             file handle for input
 * output            file handle for output
 */
typedef struct {
	unsigned char *in;
	unsigned char *old;
	char *out;
	char *out_after_offset;
	char *out_after_dump;
	char *out_eol;
	unsigned out_len;
	uint_fast64_t offset;
	uint_fast64_t processed;
	unsigned read_count;
	FILE *input;
	FILE *output;
} Private;


static void _append_ascii_col(void);
static void _append_newline(void);
static void _append_offset(void);
static void _clean(void);
static void _init(DumpState *ds, FILE *input, FILE *ouput);
static void _print_last_offset(void);
static void _read(DumpState *ds);
static void _translate_line(void);
static void _write(void);


/* private variables */
static Private private;


/* define ds, declared in "DumpState.h" */
DumpState ds = {
	.append_ascii_col = _append_ascii_col,
	.append_newline = _append_newline,
	.append_offset = _append_offset,
	.clean = _clean,
	.init = _init,
	.print_last_offset = _print_last_offset,
	.read = _read,
	.translate_line = _translate_line,
	.write = _write,
};


/* function definitions */
void
_append_ascii_col(void)
{
	while (private.out_eol < private.out_after_dump)
		*(private.out_eol++) = ' ';

	private.out_eol = append_ascii_col(private.out_after_dump, private.in,
			private.read_count);
}

void
_append_newline(void)
{
	*private.out_eol++ = '\n';
}

void
_append_offset(void)
{
	get_offset(private.out, private.offset);
}

void
_clean(void)
{
	/* clear used memory */
	memset(private.in, 0, params.width);
	memset(private.old, 0, params.width);
	memset(private.out, 0, private.out_len);

	free(private.in);
	free(private.old);
	free(private.out);
}

void
_init(DumpState *ds, FILE *input, FILE *output)
{
	ds->finished = false;
	ds->masked = false;

	private.in = _malloc(params.width);
	private.old = _malloc(params.width);

	private.out_len = OFFSET_CHAR_LEN+params.width*(type.char_width+type.space)
		+ (params.ascii_col ? params.width+4+(!type.space) : !type.space);

	private.out = _calloc(private.out_len, 1);

	private.out_after_offset = params.offset ?
		private.out+OFFSET_CHAR_LEN : private.out;
	private.out_after_dump = private.out_after_offset
		+ params.width*(type.char_width+type.space)-(type.space ? 1 : 0);

	private.out_eol = private.out;

	private.offset = params.skip;
	private.processed = 0;
	private.read_count = 0;
	private.input = input;
	private.output = output;
}

void
_print_last_offset(void)
{
	get_offset(private.out, private.offset+private.read_count);
	private.out[OFFSET_CHAR_LEN] = '\n';
	fwrite(private.out, 1, OFFSET_CHAR_LEN+1, private.output);
}

void
_read(DumpState *ds)
{
	unsigned read_len;

	/* increment offset by number of previously read bytes */
	private.offset += private.read_count;
	private.processed += private.read_count;

	read_len = (params.limited && private.processed+params.width > params.limit) ?
		params.limit-private.processed : params.width;

	private.read_count = fread(private.in, 1, read_len, private.input);

	if (!private.read_count) {
		ds->finished = true;
		return;
	} else if (private.read_count < read_len) {
		ds->masked = false; /* output last line */
		return;
	}

	/*
	 * If it is not the first line (*out == 0) or the last line
	 * (read < params.width), we check if we have to mask the output.
	 */
	if (!params.full && *private.out &&
			!memcmp(private.in, private.old, private.read_count)) {
		if (!ds->masked) {
			fputs("*\n", private.output);
			ds->masked = true;
		}
	} else {
		ds->masked = false;
	}
}

void
_translate_line(void)
{
	private.out_eol = byte_to_numeric(private.out_after_offset, private.in,
			private.read_count);
}

void
_write(void)
{
	unsigned char *tmp;

	fwrite(private.out, 1, private.out_eol-private.out, private.output);
	
	/* swap in- and old-pointer */
	tmp = private.in;
	private.in = private.old;
	private.old = tmp;
}

