@echo off
setlocal

set CC=cl
set DEBUG_FLAGS=/W4 /WX /wd4456 /wd4100 /wd4189 /Oi /Od /Zi
set RELEASE_FLAGS=/Oi /O2 /GL /GS /DNDEBUG
set LINKS=user32.lib gdi32.lib shell32.lib

set CFLAGS=
if "%1" == "release" (
    set CFLAGS=%RELEASE_FLAGS%
) else (
    set CFLAGS=%DEBUG_FLAGS%
)

cl src\main.c %CFLAGS% /Fe:breeze.exe /link %LINKS%
