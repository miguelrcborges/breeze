#ifndef C_H
#define C_H

#include "mbbase.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

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

const u16 *default_bar_font_str = L"Tahoma";

typedef struct {
	void (*fun)(void *arg);
	void *arg;
	usize line;
	u32 key;
	u32 mod;
} Hotkey;

/* action.c */
void spawn(void *arg);
void spawnWithoutConsole(void *arg);
void quit(void *arg);
void reloadConfig(void *arg);
void kill(void *arg);
void switchToDesktop(void *arg);
void sendToDesktop(void *arg);
void focusNext(void *arg);
void focusPrev(void *arg);
void revealAllWindows(void *arg);

/* config */
bool loadConfig(void);
void loadDefaultConfig(void);
void loadUserApplicationDirs(void);

/* main.c */
extern usize hotkeys_count;
extern Hotkey hotkeys_buf[MAX_HOTKEYS];
extern Hotkey *hotkeys;
extern HWND bar_window;
extern HFONT default_bar_font;
extern HFONT bar_font;
extern usize bar_font_height;
extern COLORREF background;
extern COLORREF foreground;
extern usize bar_position;
extern usize bar_pad;
extern usize bar_width;
extern RECT desktop_rect;
extern RECT hours_rect;
extern RECT minutes_rect;
extern RECT clock_rect;
extern void (*drawBar)(HDC dc);

#endif
