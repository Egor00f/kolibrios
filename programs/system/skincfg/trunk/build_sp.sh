#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	echo "lang fix es_ES"
	echo "lang fix es_ES" > lang.inc
	fasm -m 16384 skincfg.asm skincfg
	rm -f lang.inc
	exit 0




