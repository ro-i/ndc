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

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "DumpState.h"
#include "libgetopt_portable/libgetopt_portable.h"
#include "repository_definition.h"
#include "util.h"
/* last */
#include "ndc.h"


static char        *byte_to_numeric_not_power_of_two(
		char *out, const unsigned char *in, unsigned n);
static char        *byte_to_numeric_power_of_two(
		char *out, const unsigned char *in, unsigned n);
static bool         dump(FILE *input, FILE *output);
static bool         dump_reverse(FILE *input, FILE *output);
static void         init(void);
static void         limits(void);
static inline void  numeric_to_byte(
		unsigned char *out, const char *in, unsigned len);
static void         process(const char *infile, const char *outfile);
static bool         set_type(const char *name);
static int          skip_offset(FILE *f);
static void         usage(void);
static void         version(void);


/* 
 * global variables (cf. config.h, too):
 *
 * params    command line parameters
 * type      type of numeric conversion
 */
/* define "params", declared in "ndc.h" */
Params params = {
	.ascii_col = false,
	.bufsize = BUFSIZ,
	.full = false,
	.limit = 0,
	.limited = false,
	.offset = true,
	.reverse = false,
	.skip = 0,
	.width = 16
};
/* define "type", declared in "ndc.h" */
Repository type = no_repo;


/*
 * Append the ASCII representation of the "n" bytes in "in" at the end of the
 * line "out".
 *
 * return pointer to index after last character written.
 */
char *
append_ascii_col(char *out, const unsigned char *in, unsigned n)
{
	*out++ = ' ';
	*out++ = ' ';
	*out++ = '|';

#if CHAR_BIT != 8
	while (n--)
		*out++ = repo[ASC].characters[*in++%256];
#else
	while (n--)
		*out++ = repo[ASC].characters[*in++];
#endif

	*out++ = '|';
	*out++ = '\n';

	return out;
}

/*
 * Convert a byte value to its numeric string representation like "FF".
 * input: a sequence of "n" bytes
 * output: a character sequence consisting of numeric value representations
 *   like "FF" and spaces (if type.space is 1)
 * Loop over the input sequence and write to the output string in reverse
 * direction.
 *
 * return pointer to index after last character written.
 */
char *
byte_to_numeric_not_power_of_two(char *out, const unsigned char *in, unsigned n)
{
	char *end_of_line;
	int i, num;

	in += n;
	out += n*(type.char_width+type.space);

	/* no trailing space, please */
	end_of_line = type.space ? out-1 : out;

	while (n--) {
		if (type.space)
			*--out = ' ';
		for (i = type.char_width, num = *--in; i--; num /= type.base)
			*--out = type.characters[num%type.base];
	}

	return end_of_line;
}

/*
 * special (optimized) handling for numeric systems whose base are a power of two.
 */
char *
byte_to_numeric_power_of_two(char *out, const unsigned char *in, unsigned n)
{
	int shift;

	while (n--) {
		for (shift = type.start_shift; shift >= 0; shift -= type.shift)
			*out++ = type.characters[(*in >> shift) & type.mask];
		in++;
		if (type.space)
			*out++ = ' ';
	}

	return type.space ? out-1 : out;
}

/*
 * May be called multiple times if there are multiple files to process.
 *
 * one line of output consists of:
 *   - ds.offset + 2 spaces
 *   - params.width * (type.char_width+type.space) without last space
 *   - if necessary, ascii_col, consisting of:
 *     - two spaces
 *     - two pipe-symbols
 *     - params.width * ASCII-characters
 *   - newline character
 */
bool
dump(FILE *input, FILE *output)
{
	if (skip_offset(input) == EOF) {
		fprintf(output, "EOF reached after skipping %"SCNuFAST64" bytes.\n",
				params.skip);
		return true;
	}

	ds.init(&ds, input, output);

	for (;;) {
		do {
			ds.read(&ds);
		} while (ds.masked);
		if (ds.finished)
			break;

		if (params.offset)
			ds.append_offset();

		ds.translate_line();

		if (params.ascii_col)
			ds.append_ascii_col();
		else
			ds.append_newline();

		ds.write();
	}
	/* do not forget to add the previously read number of bytes to offset */
	if (params.offset)
		ds.print_last_offset();

	ds.clean();

	return ferror(input) ? false : true;
}

/*
 * Convert a string like "FF" to its byte value.
 * Try to handle incomplete numbers as if there were leading zeros (e.g.
 * consider "F" as "0F").
 * May be called multiple times if there are multiple files to process.
 */
bool
dump_reverse(FILE *input, FILE *output)
{
	char ch, *in, *ptr;
	uint_fast64_t byte_count = 0, skip = params.skip;
	unsigned char out, count = 0;

	in = _malloc(type.char_width);

	while (fread(&ch, 1, 1, input)) {
		if ((ptr = strchr(type.characters, ch))) {
			in[count++] = ptr-type.characters;
			if (count != type.char_width)
				continue;
		} else if (!strchr(skip_characters, ch) && ch) {
			die("error: invalid character -- \"%c\".", ch);
		} else if (!count) {
			continue;
		}
		if (skip) {
			skip--;
			count = 0;
			continue;
		} else if (params.limited && byte_count++ == params.limit) {
			break;
		}
		numeric_to_byte(&out, in, count);
		fputc(out, output);
		count = 0;
	}
	if (count && !skip && (!params.limited || byte_count < params.limit)) {
		numeric_to_byte(&out, in, count);
		fputc(out, output);
	}

	/* clear used memory */
	memset(in, 0, type.char_width);
	free(in);

	return ferror(input) ? false : true;
}

/*
 * Write offset - byte offset in file - in hex to the start of the string "out".
 * Append two spaces. Operate in reverse direction.
 */
void
get_offset(char *out, uint_fast64_t byte_count)
{
	out += OFFSET_CHAR_LEN-1;

	*out-- = ' ';
	*out-- = ' ';

	for (unsigned shift = 0; shift < UINT_FAST64_T_BIT_LEN; shift += 4)
		*out-- = repo[HEX_UC].characters[(byte_count >> shift) & 0xf];
}

/*
 * Calculate the length of the string representations for one byte.
 * E.g: if CHAR_BIT is "8", we need max. two hex-characters or max. three
 * decimal-characters to represent the value of one byte.
 * In order to determine "char_width", we calculate the logarithm of
 * "base" to base CHAR_MAX (round up).
 */
void
init(void)
{
	int a, i;

	if (params.reverse && type.type == ASC)
		die("Will not accept ASCII as input type.");
	else if (params.reverse && type.type == NONE)
		die("No input type specified.");
	else if (type.type == NONE)
		type = repo[HEX_UC]; /* default */

	if (!type.char_width) {
		for (a = 1, i = 1; (a *= type.base) < CHAR_MAX; i++);
		type.char_width = i;
	}

	byte_to_numeric = is_power_of_two(type.base) ?
		byte_to_numeric_power_of_two : byte_to_numeric_not_power_of_two;
}

void
limits(void)
{
	printf("max value of NUM (size_t): %zu\n"
			"max value of SIZE (uint_fast64_t): %"PRIuFAST64"\n",
			SIZE_MAX, UINT_FAST64_MAX);
}

/*
 * Convert string representation of one byte (like "FF") in "in" with length
 * "len" to its byte value in "out".
 */
void
numeric_to_byte(unsigned char *out, const char *in, unsigned len)
{
	int base = 1;

	for (*out = 0, in += len-1; len--; base *= type.base)
		*out += *in--*base;
}

/* 
 * process files (may be called multiple times)
 */
void
process(const char *infile, const char *outfile)
{
	static char *inbuf, *outbuf;
	FILE *input = stdin, *output = stdout;
	bool success = true;

	if (infile && strcmp(infile, "-")) {
		if (!(input = fopen(infile, "rb"))) {
			err("Failed to open \"%s\".", infile);
			return;
		}
		inbuf = _malloc(params.bufsize);
		if (setvbuf(input, inbuf, _IOFBF, params.bufsize))
			die("setvbuf: %s", strerror(errno));
	}
	if (outfile) {
		if (!(output = fopen(outfile, "ab")))
			die("Failed to open/create output file.");
		outbuf = _malloc(params.bufsize);
		if (setvbuf(output, outbuf, _IOFBF, params.bufsize))
			die("setvbuf: %s", strerror(errno));
	}

	if (!params.reverse) {
		fprintf(output, "Processing %s ...\n",
				input == stdin ? "stdin" : infile);
	}

	if (params.reverse)
		success = dump_reverse(input, output);
	else
		success = dump(input, output);

	if (input != stdin) {
		fclose(input);
		free(inbuf);
	} if (output != stdout) {
		fclose(output);
		free(outbuf);
	}

	if (!success)
		err("error processing %s.", input == stdin ? "stdin" : infile);
}

bool
set_type(const char *name)
{
	unsigned i;

	if (!name || !*name)
		return false;

	for (i = 0; i < TYPE_COUNT; i++) {
		if (strcmp(name, repo[i].format))
			continue;
		type = repo[i];
		return true;
	}
	return false;
}

/*
 * Do not use fseek(), as this function does not work on stdin.
 */
int
skip_offset(FILE *f)
{
	uint_fast64_t o;

	for (o = 0; o < params.skip; o++) {
		if (fgetc(f) == EOF)
			return EOF;
	}
	return 0;
}

void
usage(void)
{
	printf("usage: %s [OPTION]... [FILE]...\n"
			"Read files and do a numeric dump (e.g. hex dump) or the reverse.\n"
			"If no filename is given, read standard input.\n"
			"If standard input is specified using one dash (\"-\"),"
			" process it like a normal filename.\n"
			"\noptions:\n"
			"  -a\t\tshow ascii representation of bytes in an additional column\n"
			"  -b SIZE\tread/write using bufsize of SIZE bytes if "
			"read/write from/to a file\n"
			"  -d FILE\twrite (append) to file FILE instead of stdout\n"
			"  -f\t\tfull output - do not replace consecutive "
			"identical lines with an asterisk\n"
			"  -h\t\tshow this help\n"
			"  -l NUM\tprocess only NUM bytes\n"
			"  -L\t\tshow the limits of the numeric arguments\n"
			"  -n\t\tno offset at the beginning of every line of output\n"
			"  -r\t\treverse mode: translate string representations of numeric"
			" values to bytes\n"
			"\t\t\tTabs, spaces and newlines are silently skipped.\n"
			"\t\t\trequires \"-t\" option\n"
			"  -s NUM\tskip first NUM bytes of every input file (or stdin)\n"
			"  -t TYPE\tset numeric system to TYPE\n"
			"\t\t\tavailable types are:\n"
			"\t\t\t\tX (hexadecimal uppercase) (default)\n"
			"\t\t\t\ta (ASCII)\n"
			"\t\t\t\tb (binary)\n"
			"\t\t\t\td (decimal)\n"
			"\t\t\t\to (octal)\n"
			"\t\t\t\tx (hexadecimal lowercase)\n"
			"  -v\t\tshow version information\n"
			"  -w WIDTH\tdisplay WIDTH bytes per line (arbitrary limit: 256)\n"
			"\nnotes:\n"
			"Use -L to see the limits of the numeric arguments on your system.\n",
		NAME_STR
			);
}

void
version(void)
{
	printf("%s, version %s\n"
			"Copyright (C) 2019-2020 Robert Imschweiler.\n"
			"License GPLv3+: GNU GPL version 3 or later "
			"<https://gnu.org/licenses/gpl.html>\n"
			"This is free software; you are free to change and "
			"redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n",
			NAME_STR, VERSION_STR
	      );
}

int
main(int argc, char * const *argv)
{
	const char *outfile = NULL;
	int opt;

	while ((opt = getopt_portable(argc, argv, "ab:d:fhl:Lnrs:t:vw:")) != -1) {
		switch (opt) {
		case 'a':
			params.ascii_col = true;
			break;
		case 'b':
			if (sscanf(opt_arg, "%zu", &params.bufsize) <= 0
					|| !params.bufsize)
				die("option '%c' -- invalid size: %s", opt, opt_arg);
			break;
		case 'd':
			outfile = opt_arg;
			break;
		case 'f':
			params.full = true;
			break;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		case 'l':
			if (strchr(opt_arg, '-')
					|| sscanf(opt_arg, "%"SCNuFAST64, &params.limit) <= 0)
				die("option '%c' -- invalid size: %s", opt, opt_arg);
			params.limited = true;
			break;
		case 'L':
			limits();
			return EXIT_SUCCESS;
		case 'n':
			params.offset = false;
			break;
		case 'r':
			params.reverse = true;
			break;
		case 's':
			if (strchr(opt_arg, '-')
					|| sscanf(opt_arg, "%"SCNuFAST64, &params.skip) <= 0)
				die("option '%c' -- invalid size: %s", opt, opt_arg);
			break;
		case 't':
			if (!set_type(opt_arg))
				die("option '%c' -- unsupported type: %s", opt, opt_arg);
			break;
		case 'v':
			version();
			return EXIT_SUCCESS;
		case 'w':
			if (strchr(opt_arg, '-')
					|| sscanf(opt_arg, "%u", &params.width) <= 0
					|| !params.width || params.width > 256)
				die("option '%c' -- invalid size: %s", opt, opt_arg);
			break;
		default:
			usage();
			return EXIT_FAILURE;
		}
	}

	init();

	if (opt_ind < argc) {
		/* process all files */
		do {
			process(argv[opt_ind++], outfile);
		} while (opt_ind < argc);
	} else {
		/* read stdin */
		process(NULL, outfile);
	}

	return EXIT_SUCCESS;
}
