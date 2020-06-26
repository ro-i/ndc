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
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"


static void  verr(const char *format, va_list ap);


void *
_calloc(size_t n, size_t s)
{
	void *p;

	p = calloc(n, s);
	if (!p)
		die("calloc: %s", strerror(errno));

	return p;
}

void *
_malloc(size_t n)
{
	void *p;

	p = malloc(n);
	if (!p)
		die("malloc: %s", strerror(errno));

	return p;
}

void
die(const char *format, ...)
{
	va_list args;

	if (format) {
		va_start(args, format);
		verr(format, args);
		va_end(args);
	}
	exit(EXIT_FAILURE);
}

void
err(const char *format, ...)
{
	va_list args;

	if (format) {
		va_start(args, format);
		verr(format, args);
		va_end(args);
	}
}

void
verr(const char *format, va_list ap)
{
	fprintf(stderr, "%s: ", NAME_STR);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);
}

/* cf. https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2 */
bool
is_power_of_two(uint_fast64_t n)
{
	return n && !(n & (n-1));
}

