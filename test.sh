#!/bin/sh

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

# Note:
# If "ndc" works, it has to be possible to dump a binary file and convert it
# back to binary without changing anything.


## functions ##


before_test () {
	printf '' > "$dump"
	printf '' > "$binary"
}

check_diff () {
	local result

	diff -q "$file" "$binary" > /dev/null
	result=$?

	check_result $result
}

check_invalid_size_error () {
	printf '%s\n' "$*" | sed '1q'
	# check for error message
	printf '%s\n' "$*" | grep -q "invalid size"

	check_result $?
}

check_result () {
	if [ "$1" -eq 0 ]; then
		print_green "$file: test $current_test_name passed."
	else
		print_red "$file: test $current_test_name failed."
		success=false
	fi
}

# compare two integer values and check if the first one is smaller
is_smaller () {
	local smaller

	[ "$1" = "$2" ] && { false; return; }

	smaller=$(printf '%s\n%s\n' "$1" "$2" | sort -n | sed '1q')

	[ "$smaller" = "$1" ]
}

# remove unwanted lines from dump before do the reverse operation
prepare_dump_for_reverse_operation () {
	# remove the "Processing [file] ..." thing, i.e. the first line
	sed -i '1d' "$dump"
	# remove the "EOF reached [...]" thing
	sed -i '/^EOF.*/d' "$dump"
}

print_color () {
	local color_code="$1"
	shift
	if $color; then
		printf '\033[01;3'"$color_code"'m%s\e[0m\n' "$*"
	else
		printf '%s\n' "$*"
	fi
}

print_blue () {
	print_color 4 "$*"
}

print_green () {
	print_color 2 "$*"
}

print_red () {
	print_color 1 "$*"
}

print_yellow () {
	print_color 3 "$*"
}

usage () {
	printf '%s\n' "usage: $0 [OPTION]... TEST NDC_EXECUTABLE FILE..."
	printf 'options available:
    --color          colored output (default)
    -h, --help       show this help
    --nocolor        no colored output
    --valgrind       execute test using valgrind
  tests available:
    check_format_ascii  check for correct number of ascii-characters ("-a" option)
    check_offset_value  check correct last offset value (= file size)
    default             default test set
    reverse             check dump+reverse == original file
    skip_limit          only tests -l and -s options
    width               only tests -w option\n'
}


## test functions ##


check_format_ascii () {
	current_test_name="check_format_ascii"

	before_test

	width=$(shuf -n1 -i 1-256)
	default_dump_cmd -aw"$width"

	prepare_dump_for_reverse_operation
	
	odd_lines=$(grep -cEvx '[^ ]+( [^ ]+){'"$((width-1))"'}  \|.{'"$width"'}\|' "$dump")

	if [ "$odd_lines" -gt 1 ]; then
		print_red "$file: test $current_test_name failed."
		success=false
	else
		print_green "$file: test $current_test_name passed."
	fi
}

check_offset_value () {
	current_test_name="check_offset_value"

	before_test

	# without "-n"
	printf '%s\n' "${debug_cmd}\"$bin\" -f -d \"$dump\" -t $type \"$file\""
	$debug_cmd "$bin" -f -d "$dump" -t "$type" "$file"

	# get file size in hex (uppercase)
	size=$(printf '%X' "$(stat -Lc '%s' "$file")")

	# get file size from ndc (last offset)
	ndc_size=$(tail -n1 "$dump" | cut -d' ' -f1 | sed 's/^0*//')

	if [ "$size" != "$ndc_size" ]; then
		print_red "$file: test $current_test_name failed; size should "\
			"be $size, but was $ndc_size."
		success=false
	else
		print_green "$file: test $current_test_name passed."
	fi
}

default () {
	check_format_ascii
	check_offset_value
	reverse
	skip_limit
	width
}

default_dump_cmd () {
	printf '%s\n' "${debug_cmd}\"$bin\" -n -f -d \"$dump\" -t $type $* \"$file\""
	$debug_cmd "$bin" -n -f -d "$dump" -t "$type" "$@" "$file"
}

default_reverse_cmd () {
	printf '%s\n' "${debug_cmd}\"$bin\" -d \"$binary\" -t $type -r $* \"$dump\""
	$debug_cmd "$bin" -d "$binary" -t "$type" -r "$@" "$dump"
}

reverse () {
	current_test_name="reverse"

	before_test

	# dump binary
	default_dump_cmd

	prepare_dump_for_reverse_operation

	# reverse operation
	default_reverse_cmd

	check_diff
}

skip_limit_intern () {
	skip="$1"
	limit="$2"

	# get file size
	size=$(stat -Lc '%s' "$file")

	# dump binary
	default_dump_cmd -s "$skip" -l "$limit"

	prepare_dump_for_reverse_operation

	# reverse operation
	# get first chunk of file (previously skipped)
	printf '%s\n' "head --bytes=$skip \"$file\" >> \"$binary\""
	head --bytes="$skip" "$file" >> "$binary"
	# append translated chunk of file
	default_reverse_cmd
	# append non-translated chunk of file (after limit)
	if [ $((skip+limit)) -gt 0 ] && [ $((size-$((skip+limit)))) -gt 0 ]; then
		printf '%s\n' "tail --bytes=$((size-$((skip+limit)))) "\
			"\"$file\" >> \"$binary\""
		tail --bytes=$((size-$((skip+limit)))) "$file" >> "$binary"
	fi

	check_diff
}

skip_limit () {
	current_test_name="skip_limit"

	# test random values
	for i in $(seq 1 10); do
		before_test
		skip_limit_intern "$(shuf -n1 -i 0-32768)" "$(shuf -n1 -i 0-32768)"
	done

	skip_limit_edge
}

skip_limit_edge () {
	current_test_name="skip_limit (edge)"

	random=$(shuf -n1 -i 0-32768)
	before_test
	check_invalid_size_error "$(default_dump_cmd -s -1 -l "$random" 2>&1)"
	before_test
	check_invalid_size_error "$(default_dump_cmd -s "$random" -l -1 2>&1)"
	before_test
	skip_limit_intern 0 "$random"
	before_test
	skip_limit_intern "$random" 0
}

width_intern () {
	if [ "$1" -lt 1 ] || [ "$1" -gt 256 ]; then
		check_invalid_size_error "$(default_dump_cmd -w "$1" 2<&1)"
		return;
	fi

	# dump binary
	default_dump_cmd -w "$1"

	prepare_dump_for_reverse_operation

	# check correct number of columns
	width=$(sed '1q' "$dump" | wc -w)
	odd_lines=$(grep -cEvx '[^ ]+( [^ ]+){'"$((width-1))"'}' "$dump")
	# get number of lines with different number of columns
	if [ "$width" -ne "$1" ]; then
		print_red "$file: test $current_test_name failed; width should "\
			"be $1, but was $width."
		success=false
	elif [ "$odd_lines" -gt 1 ]; then
		print_red "$file: test $current_test_name failed; not all lines "\
			"are correctly formatted."
		success=false
	else
		print_green "$file: test $current_test_name passed."
	fi
}

# width option is (arbitrarily) limited to 256
width () {
	current_test_name="width"

	# test random values
	for i in $(seq 1 10); do
		before_test
		width_intern "$(shuf -n1 -i 1-256)"
	done

	width_edge
}

width_edge () {
	current_test_name="width (edge)"

	for i in -1 0 256 257; do
		before_test
		width_intern $i
	done
}


## functions end ##


color=true
current_test_name=""
success=true
valgrind_cmd="valgrind -q --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all "
debug_cmd=""

if [ $# -le 2 ]; then
	usage; exit 1
fi

# parse options
while [ -n "${1%%[^-]*}" ]; do
	if [ "$1" = "--color" ]; then
		color=true
	elif [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
		usage; exit 0
	elif [ "$1" = "--nocolor" ]; then
		color=false
	elif [ "$1" = "--valgrind" ]; then
		if ! type valgrind > /dev/null 2>&1; then
			print_red "Could not find valgrind."
			exit 1
		fi
		debug_cmd="$valgrind_cmd"
	else
		usage; exit 1
	fi
	shift
done

test_option="$1"
case "$test_option" in
	"check_format_ascii")
		test_cmd () { check_format_ascii; };;
	"check_offset_value")
		test_cmd () { check_offset_value; };;
	"default")
		test_cmd () { default; };;
	"reverse")
		test_cmd () { reverse; };;
	"skip_limit")
		test_cmd () { skip_limit; };;
	"width")
		test_cmd () { width; };;
	*)
		usage; exit 1;;
esac
shift

bin="$1"
if [ ! -r "$bin" ] || [ ! -x "$bin" ]; then
	print_red "$bin: file not found or wrong permissions."
	exit 1
else
	shift
fi


binary=$(mktemp)
dump=$(mktemp)
if [ ! -w "$binary" ] || [ ! -w "$dump" ]; then
	print_red "Could not create temporary files or files not writable."
	exit 1
fi

for file in "$@"; do
	if [ ! -r "$file" ]; then
		print_yellow "$file: cannot read"
		continue
	fi

	# Note that ASCII cannot be converted back to binary file.
	for type in d b o x; do
		before_test
		test_cmd
	done
done

rm "$dump" "$binary"

if $success; then
	print_green 'All tests passed.'
fi
