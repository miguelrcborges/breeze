LINKS="-luser32"
BIN="kbswtch.exe"

if [ -z "$CC" ]
then
	CC="clang"
fi

if [ -z "$CFLAGS" ]
then
	CFLAGS="-O2 -flto -s -D_FORTIFY_SOURCE=1 -pipe"
fi

$CC $CFLAGS main.c $LINKS -o $BIN
