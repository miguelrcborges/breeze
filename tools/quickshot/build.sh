LINKS="-ffreestanding -nostdlib -lgdi32 -luxtheme -lmsimg32 -luser32 -lkernel32 -mwindows"
BIN="quickshot.exe"

if [ -z "$CC" ]
then
	CC="clang"
fi

if [ -z "$CFLAGS" ]
then
	CFLAGS="-O2 -flto -s -D_FORTIFY_SOURCE=1 -pipe"
fi

$CC $CFLAGS main.c $LINKS -o $BIN
