#ifndef C_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define unused(x) (void)(x)

#include "mbbase.h"


#define MAX_MONITORS 8
#define MAX_WORKSPACES 10
#define MAX_WINDOWS 2048
#define MAX_HOTKEYS 1024
#define MAX_IGNORE_WINDOWS 512
#define MAX_PLUGINS 32

typedef struct BreezeState BreezeState;

#define HotkeyFunction(f) void f(BreezeState *state, uptr arg)
typedef HotkeyFunction(HotkeyFunction);

#define BreezePluginSetupFunction(f) void f(BreezeState *state)
typedef BreezePluginSetupFunction(BreezePluginSetupFunction);

#define BreezePluginCleanupFunction(f) void f(void)
typedef BreezePluginCleanupFunction(BreezePluginCleanupFunction);

#define BreezePluginWorkspaceChangeCallback(f) void f(i8 new_workspace)
typedef BreezePluginWorkspaceChangeCallback(BreezePluginWorkspaceChangeCallback);

#define HK_MOD_SHIFT ((u16) (1u << 8))
#define HK_MOD_CTRL ((u16) (1u << 9))
#define HK_MOD_ALT ((u16) (1u << 10))
#define HK_MOD_WIN ((u16) (1u << 11))

#define WINDOW_TILED ((u16) (1u << 10))

#define PLUGIN_BAR ((u16) (1u << 0))


typedef struct i8x2 {
	i8 x;
	i8 y;
} i8x2;

typedef struct i32x2 {
	i32 x;
	i32 y;
} i32x2;

typedef struct Window {
	HWND handle;
	u16 next_workspace_window;
	u16 prev_workspace_window;
	i8 monitor_index;
	i8 workspace;
} Window;

typedef struct Hotkey {
	u16 hotkey_value;
	HotkeyFunction *function;
	uptr argument;
} Hotkey;

typedef struct Monitor {
	i32 width;
	i32 height;
	i32 dpi;
	RECT work_area;
} Monitor;

struct BreezeState {
	HINSTANCE hInstance;
	i8 current_workspace;
	struct {
		HWND buffer[MAX_IGNORE_WINDOWS];
		i32 count;
	} IgnoreWindows;
	struct {
		Monitor buffer[MAX_MONITORS];
		i8 main_monitor_index;
		i32 count;
	} Monitors;
	struct {
		Window buffer[MAX_WINDOWS];
		u16 first_free_index;
		i32 buffer_alloc_pos;
		HWND last_focused_window[MAX_WORKSPACES];
		u16 current_workspace_window[MAX_MONITORS][MAX_WORKSPACES];
	} Windows;
	struct {
		Hotkey buffer[MAX_HOTKEYS];
		i32 count;
		u16 modifiers_state;
	} Hotkeys;
	u16 PluginFlags;
	struct {
		struct {
			HMODULE dll_module;
			BreezePluginCleanupFunction *cleanup_function;
		} buffer[MAX_PLUGINS];
		BreezePluginWorkspaceChangeCallback *workspace_change_callback;
		i32 count;
	} Plugins;
};


/* config */
static void LoadConfig(BreezeState *state, uptr unused);

/* actions */
static HotkeyFunction(ActionSpawn);
static HotkeyFunction(ActionKill);
static HotkeyFunction(ActionQuit);
static HotkeyFunction(ActionSetWorkspace);
static HotkeyFunction(ActionSendToWorkspace);

#define C_H
#endif
