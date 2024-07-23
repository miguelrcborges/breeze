LINKS="-luser32 -lgdi32"

if [ -z "$CC" ]
then
	CC="clang"
fi

if [ -z "$CFLAGS" ]
then
	CFLAGS="-O2 -flto -s -D_FORTIFY_SOURCE=1"
fi

CFLAGS="-DWINDOW -mwindows"

$CC $LINKS $CFLAGS src/main.c -o breezew-mingw-ucrt.exe
