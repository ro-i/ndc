ndc - numeric dump and conversion
=================================
```
ndc - numeric dump and conversion
Copyright (C) 2019-2020 Robert Imschweiler

This file is part of ndc.

ndc is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ndc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ndc.  If not, see <https://www.gnu.org/licenses/>.
```

ndc is a program to dump files to hexadecimal or other numeric systems and vice
versa.

Project Status
--------------
I wrote this software some time ago in order to learn something new and 
in order to use it and to play with it. However, I am no longer satisfied
with the current state of the project. That's why it is now archived for the
time being. :)

important features/goals:
------------------------
* own conversion algorithm
* variable-sized io-buffer (defaults to `BUFSIZ`)
* consecutive identical lines are replaced by an asterisk (may be turned off
  via command-line option `-f`)
* portability - even for non-POSIX systems
  (That is why an own implementation of `getopt()` is used.)
  (+ does not require one byte to consist of eight bits - relies on CHAR_BIT)
* be fast ;-)

Dependencies
-------------
none

Coding style
-------------
My coding style is very similar to the suggestions of the "suckless" community
(https://suckless.org/coding_style/ and https://suckless.org/philosophy/) and
the Linux kernel (https://www.kernel.org/doc/Documentation/process/coding-style.rst).

Project architecture
---------------------
### headers:
* `DumpState.h`: declaration of DumpState object
* `ndc.h`: function and variable declarations for `ndc.c`
* `repository.h`/`repository_definition.h`: static data for numeric conversion
* `util.h`: function and variable declarations for `util.c`
### source files:
* `DumpState.c`: definition of DumpState object
* `ndc.c`: main source of ndc
* `util.c`: some functions that have nothing to do with the actual functionality
            of the program
### tests:
* `test.sh`: If `ndc` works, it has to be possible to dump a binary file and convert
    it back to binary without changing anything. Use `make test` to run the default
    test set. Run `sh ./test -h` to view every option currently available.

Help output (option `-h`)
--------------------------
```
usage: ndc [OPTION]... [FILE]...
Read files and do a numeric dump (e.g. hex dump) or the reverse.
If no filename is given, read standard input.
If standard input is specified using one dash ("-"), process it like a normal filename.

options:
  -a		show ascii representation of bytes in an additional column
  -b SIZE	read/write using bufsize of SIZE bytes if read/write from/to a file
  -d FILE	write (append) to file FILE instead of stdout
  -f		full output - do not replace consecutive identical lines with an asterisk
  -h		show this help
  -l NUM	process only NUM bytes
  -L		show the limits of the numeric arguments
  -n		no offset at the beginning of every line of output
  -r		reverse mode: translate string representations of numeric values to bytes
			Tabs, spaces and newlines are silently skipped.
			requires "-t" option
  -s NUM	skip first NUM bytes of every input file (or stdin)
  -t TYPE	set numeric system to TYPE
			available types are:
				X (hexadecimal uppercase) (default)
				a (ASCII)
				b (binary)
				d (decimal)
				o (octal)
				x (hexadecimal lowercase)
  -v		show version information
  -w WIDTH	display WIDTH bytes per line (arbitrary limit: 256)

notes:
Use -L to see the limits of the numeric arguments on your system.
```
