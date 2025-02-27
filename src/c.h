#ifndef C_H
#define C_H

#include "mbbase.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#define unused(x) (void)(x)

enum CONSTANTS {
	MAX_HOTKEYS = 1024,
	MAX_CONFIG_FILE_SIZE = 65535,
	WIDESTR_ALLOC_BUFF_SIZE = 10240, 

	MAX_DESKTOPS = 10,
	MAX_WINDOWS_PER_DESKTOP = 2048,

	BAR_INVALIDATE_CLOCK_DURATION = 5000, // milis
	BAR_DEFAULT_WIDTH = 48,
	BAR_DEFAULT_PAD = 16,
	BAR_DEFAULT_FONT_HEIGHT = 26,
	BAR_DEFAULT_BACKGROUND = 0x181818, // bbggrr
	BAR_DEFAULT_FOREGROUND = 0xefe4e4, // bbggrr
};

enum BAR_POSITIONS {
	BAR_LEFT,
	BAR_TOP,
	BAR_RIGHT,
	BAR_BOTTOM
};

typedef struct BreezeState BreezeState;
typedef void PluginSetupFunction(BreezeState *state);
typedef void DrawBarFunction(BreezeState *state);
typedef void PluginCleanupFunction(BreezeState *state);
typedef void HotkeyFunction(BreezeState *state, void *arg);

typedef struct {
	HotkeyFunction *fun;
	void *arg;
	u32 line;
	u32 key;
	u32 mod;
} Hotkey;


struct BreezeState {
	struct {
		Hotkey *current;
		Hotkey *buffer;
		usize length;
		usize capacity;
	} hotkeys;
	struct {
		DrawBarFunction *draw_function;
		HWND window;
		HFONT default_font;
		HFONT current_font;
		COLORREF background;
		COLORREF foreground;
		DWORD font_height;
		u16 padding;
		u16 width;
		u8 position;
	} bar;
	struct {
		u16 *buffer;
		usize length;
		usize capacity;
	} widestring_allocator;
	HMODULE breeze_plugin;
	void *plugin_data;
	usize current_desktop;
};

#define InsertValue(vector, value, breezeState) do { \
		if (vector.length == vector.capacity) { \
			vector.buffer = realloc(vector.buffer, vector.capacity * (2 * sizeof(vector.buffer[0]))); \
			if (vector.buffer == NULL) { \
				MessageBoxA(NULL, "Went out of memory, exiting.", "Breeze error", MB_ICONERROR | MB_OK); \
				quit(breezeState, (void *)1); \
			} \
			vector.capacity *= 2; \
		} \
		vector.buffer[vector.length] = value; \
		vector.length += 1; \
	} while(0);

static const u16 *default_bar_font_str = L"Tahoma";


/* action.c */
void spawn(BreezeState *state, void *arg);
void spawnWithoutConsole(BreezeState *state, void *arg);
void quit(BreezeState *state, void *arg);
void reloadConfig(BreezeState *state, void *arg);
void kill(BreezeState *state, void *arg);
void switchToDesktop(BreezeState *state, void *arg);
void sendToDesktop(BreezeState *state, void *arg);
void focusNext(BreezeState *state, void *arg);
void focusPrev(BreezeState *state, void *arg);
void revealAllWindows(BreezeState *state, void *arg);

/* config */
void loadConfig(BreezeState *breezeState);
void loadUserApplicationDirs(void);

/* default_bars.c */
void drawVertical24hClock(BreezeState *breezeState);
void drawHorizontal24hClock(BreezeState *breezeState);


#endif
