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

#ifndef NDC_H
#define NDC_H

/*
 * length of offset representation in hex characters
 *   = number of bits in uint_fast64_t / 4
 *   (round up!, i.e., add 3 before division)
 *   + 2 spaces
 */
#define OFFSET_CHAR_LEN        ((sizeof(uint_fast64_t)*CHAR_BIT+3)/4+2)
/* length of offset representation in bits */
#define UINT_FAST64_T_BIT_LEN  (sizeof(uint_fast64_t)*CHAR_BIT)


/*
 * ascii_col              print ascii representation as little column after
 *                          numeric representation?
 * bufsize                size of the chunks we read
 * full                   full output - do not replace consecutive identical
 *                          lines with an asterisk (defaults to false)
 * limit                  stop after n bytes (applies only if "limited" is set)
 * limited                respect "limit"
 * offset                 wether to display the offset at the beginning of every
 *                          line of output (default=true)
 * reverse                translate numeric system -> bytes (defaults to false)
 * skip                   skip n bytes
 * type                   numeric system to use to encode input or decode input
 *                          defaults to hex (uppercase) - cf. init()
 * width                  number of bytes to display per line
 */
typedef struct {
	bool               ascii_col;
	size_t             bufsize;
	bool               full;
	uint_fast64_t      limit;
	bool               limited;
	bool               offset;
	bool               reverse;
	uint_fast64_t      skip;
	unsigned           width;
} Params;


/* functions */
char *append_ascii_col(char *out, const unsigned char *in, unsigned n);
void  get_offset(char *out, uint_fast64_t byte_count);

/* function pointer */
extern char * (*byte_to_numeric)(char *out, const unsigned char *in, unsigned n);

/* variables */
extern Params params;
extern Repository type;

#endif /* NDC_H */
