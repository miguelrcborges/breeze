# breeze

A minimalistic windows shell.


### Configuration

Configuration can be done by creating a `breeze.conf` file alongside the binary.
The configuration is pretty simple. An example one can be seen [here](breeze.conf).


### Default Keybinds

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
- [ ] Override Windows taken hotkeys (through the use of a LLK Hook)
- [ ] Custom bar drawing as a plugin (through the use of dynamic linking)
- [ ] Wallpaper support
- [ ] Dynamic Dispatch DPI awareness and act acordingly
- [ ] Making it CRT free (to consider)
