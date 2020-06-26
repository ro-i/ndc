# ndc - numeric dump and conversion
# Copyright (C) 2019-2020 Robert Imschweiler
#
# This file is part of ndc.
#
# ndc is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ndc is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ndc.  If not, see <https://www.gnu.org/licenses/>.
.POSIX:

include config.mk

bin = $(name_str)
src = $(name_str).c util.c libgetopt_portable/libgetopt_portable.c DumpState.c
obj = ${src:.c=.o}
files = COPYING README.md Makefile config.mk $(hdr) $(src) $(bin).1 test.sh \
	libgetopt_portable/COPYING libgetopt_portable/README.md \
	libgetopt_portable/changelog

all: $(bin)

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(bin)

debug_no_opt: CFLAGS = $(CFLAGS_DEBUG_NO_OPT)
debug_no_opt: $(bin)

profiling0: LDFLAGS += $(LDFLAGS_PROFILING)
profiling0: CFLAGS = $(CFLAGS_PROFILING0)
profiling0: $(bin)

profiling1: LDFLAGS += $(LDFLAGS_PROFILING)
profiling1: CFLAGS = $(CFLAGS_PROFILING1)
profiling1: $(bin)

profiling2: LDFLAGS += $(LDFLAGS_PROFILING)
profiling2: CFLAGS = $(CFLAGS_PROFILING2)
profiling2: $(bin)

profiling3: LDFLAGS += $(LDFLAGS_PROFILING)
profiling3: CFLAGS = $(CFLAGS_PROFILING3)
profiling3: $(bin)

$(bin): $(src)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(bin) $^

clean:
	rm -f $(bin) $(obj) test

dist:
	tar -czvf $(bin)_$(version_str).orig.tar.gz $(files)

install:
	install $(bin) $(DESTDIR)$(prefix_dir)/bin
	install $(bin).1 $(DESTDIR)$(prefix_dir)/share/man/man1
	gzip $(DESTDIR)$(prefix_dir)/share/man/man1/$(bin).1

test:
	cp test.sh test
	chmod a+x test
	@echo Running test...
	/bin/sh ./test default $(DESTDIR)$(prefix_dir)/bin/$(bin) \
		$(DESTDIR)$(prefix_dir)/bin/$(bin)

.PHONY: all clean debug dist install test
