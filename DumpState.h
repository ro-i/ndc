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

#ifndef DUMPSTATE_H
#define DUMPSTATE_H

/*
 * finished            all work done
 * masked              if set, output is masked with an asterisk
 * append_ascii_col()  append ascii_col to output line
 * append_newline()    append newline to output line
 * append_offset()     append offste to output line
 * init()              allocate and initialize the object structures
 * read()              read new bytes
 * print_last_offset() output last offset value (= file size) in own line
 * translate_line()    translate bytes to numeric output string
 * write()             write output line
 * clean()             free all allocated space
 */
typedef struct DumpState DumpState;
struct DumpState {
	bool finished;
	bool masked;
	void (*append_ascii_col)(void);
	void (*append_newline)(void);
	void (*append_offset)(void);
	void (*clean)(void);
	void (*init)(DumpState *ds, FILE *input, FILE *output);
	void (*print_last_offset)(void);
	void (*read)(DumpState *ds);
	void (*translate_line)(void);
	void (*write)(void);
};

extern DumpState ds;


#endif /* DUMPSTATE_H */
