#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	echo "lang fix en_US"
	echo "lang fix en_US" > lang.inc
	mkdir bin
	fasm -m 16384 rtfread.asm ./bin/rtfread
	rm -f lang.inc
	exit 0




