#!/bin/bash

CC="${CC:-clang}"
DEBUG_FLAGS="-Wall -Wextra -Werror -Og -g -gcodeview -Wl,--pdb=breeze.pdb"
RELEASE_FLAGS="-O2 -flto -s -D_FORTIFY_SOURCE=1"
LINKS="-luser32 -lgdi32 -mwindows"


if [ "$1" = "release" ]; then
    CFLAGS="${CFLAGS:-$RELEASE_FLAGS}"
else
    CFLAGS="${CFLAGS:-$DEBUG_FLAGS}"
fi

$CC $LINKS $CFLAGS src/main.c -o breeze.exe
