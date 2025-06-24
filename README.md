# breeze

A minimalistic windows shell.

**NOTE**: In the process of rework. Such was needed for the introdution of newer features and it also allows a better plugin API.

## Rework Changes

- Keybinds systems is now implement through a keyboard hook, making it possible to override keybinds that were prevously reserved by windows.
- Multiple plugins support, so there isn't the need to merge code of different plugins to merge behaviour.
- Better plugin API.

Wallpaper support is planned to be supported as a plugin, maintained by me alongside the main utility.


# Building

Get any compiler toolchain. My recommendation is [Clang MinGW](https://github.com/mstorsjo/llvm-mingw).


## GNU-style compilers

Run: 
```
clang -O2 -flto -s -D_FORTIFY_SOURCE=1 src/main.c -luser32 -lgdi32 -mwindows
```

You can replace clang with any compiler (for example, `tcc`, `gcc`, etc.).


## MSVC 

On the developer console, run:

```
build.bat release
```
