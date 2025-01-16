# breeze

A minimalistic windows shell.


### Configuration

Configuration can be done by creating a `breeze.conf` file alongside the binary.
The configuration is pretty simple. An example one can be seen [here](breeze.conf).


# Building

Get any compiler toolchain. My recommendation is [Clang MinGW](https://github.com/mstorsjo/llvm-mingw).


## Clang/GCC with bash

Run: 
```
sh build.sh release
```

By default tries to run with clang. You can change the compiler by setting the CC variable:
```
CC=gcc sh build.sh release
```


## MSVC 

On the developer console, run:

```
build.bat release
```


## Others

If you don't use `bash` with `clang` or `gcc`, or in case you want to try to compile with other compilers, do: 

```
clang -O2 -flto -s -D_FORTIFY_SOURCE=1 src/main.c -luser32 -lgdi32 -mwindows
```

Feel free to replace `clang` and the flags with whatever you want.




# Default Keybinds

- `win + e` - Opens explorer's file explorer.
- `win + r` - Reloads user's configuration.
- `ctrl + alt + shift + q` - Stops breeze and restarts explorer. 
- `win + c` - Kill focused window.
- `ctrl + enter` - Spawn terminal.
- `win + a` - Open user applications folder.
- `win + shift + a` - Open system applications folder.
- `win + [0-9]` - Go to virtual desktop n.
- `win + shift + [0-9]` - Send focused window to virtual desktop n.



### Roadmap

- [X] Virtual Destkops (remains adding to configuration)
- [X] System bar (clock / virtual desktop id)
- [X] Application Launcher (separate tool)
- [X] Dynamic Dispatch DPI awareness and act acordingly
- [X] Custom bar drawing as a plugin (through the use of dynamic linking)
    - You can override `BreezeState`'s `bar.draw_function` function pointer during `BreezePluginSetup`.
- [ ] Override Windows taken hotkeys (through the use of a LLK Hook)
- [ ] Wallpaper support
