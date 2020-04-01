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

#ifndef REPOSITORY_H
#define REPOSITORY_H


/* supported types of numeric systems */
enum Type {
	ASC,
	BIN,
	DEC,
	HEX_LC,
	HEX_UC,
	OCT,
	TYPE_COUNT,
	NONE
};

/*
 * Repository
 *
 * type        - cf. enum above
 * format      - format string for command line option "-t"
 * char_width  - maximum length of unit representation for one byte
 *                 e.g.:
 *                   assuming 8 bits / byte, we need 2 characters to display it
 *                   in hex (e.g. "FF") or max. 3 characters to display it in
 *                   dec (e.g. "123")
 *               If char_width is initialized to "0", it is calculated at
 *               runtime by init_types()
 * space       - whether to draw a space between the units
 *               e.g.:
 *                 we want "FF 68 09", but not "h e l l o" instead of "hello"
 *               can be "0" or "1"
 * characters  - the characters to use as lookup table - order matters!
 * base        - the base (e.g. hex: 16, dec: 10)
 *
 * "start_shift", "shift", "mask" are attributes for numeric systems whose base
 * are a power of two. For all other systems, these attributes are ignored.
 */
typedef struct {
	unsigned type;
	char *format;
	unsigned char_width;
	unsigned space;
	char *characters;
	unsigned base;
	unsigned start_shift;
	unsigned shift;
	unsigned mask;
} Repository;

#endif /* REPOSITORY_H */
