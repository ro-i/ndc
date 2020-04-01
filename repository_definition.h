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
#ifndef REPOSITORY_DEFINITION_H
#define REPOSITORY_DEFINITION_H


#include "repository.h"


#define START_SHIFT(shift) (CHAR_BIT%(shift) ? \
		CHAR_BIT-(CHAR_BIT%(shift)) : CHAR_BIT-(shift))


#define ASC_CHARS (\
		"................................ !\"#$%&'()*+,-./0123456789:"\
		";<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu"\
		"vwxyz{|}~."\
		"............................................................"\
		"............................................................"\
		"........"\
		)
#define BIN_CHARS ("01")
#define DEC_CHARS ("0123456789")
#define HEX_LC_CHARS ("0123456789abcdef")
#define HEX_UC_CHARS ("0123456789ABCDEF")
#define OCT_CHARS ("01234567")


/* NOTE: Must respect the order in the enum above! */
static Repository repo[TYPE_COUNT] = {
	{ ASC,    "a",  1,        0, ASC_CHARS,    256, START_SHIFT(8), 8, 0xff },
	{ BIN,    "b",  CHAR_BIT, 1, BIN_CHARS,    2,   START_SHIFT(1), 1, 0x1  },
	{ DEC,    "d",  0,        1, DEC_CHARS,    10,  0, 0, 0 },
	{ HEX_LC, "x",  0,        1, HEX_LC_CHARS, 16,  START_SHIFT(4), 4, 0xf  },
	{ HEX_UC, "X",  0,        1, HEX_UC_CHARS, 16,  START_SHIFT(4), 4, 0xf  },
	{ OCT,    "o",  0,        1, OCT_CHARS,    8,   START_SHIFT(3), 3, 0x7  },
};

static const Repository no_repo = { NONE, "", 0, 0, NULL, 0, 0, 0, 0 };


#endif /* REPOSITORY_DEFINITION_H */
