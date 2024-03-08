LINKS="-lkernel32 -luser32"

if [ -z "$CC" ]
then
	CC="clang"
fi

if [ -z "$CFLAGS" ]
then
	CFLAGS="-O2 -flto -s -D_FORTIFY_SOURCE=1"
fi

CFLAGS="$CFLAGS -ffreestanding -nostdlib -Ideps/lib"

$CC $LINKS $CFLAGS src/*.c src/config/*.c deps/lib/src/win32/io.c deps/lib/src/win32/mem.c deps/lib/src/compilercope.c deps/lib/src/mem.c deps/lib/src/str.c deps/lib/src/win32/chkstk.S -o breeze.exe
