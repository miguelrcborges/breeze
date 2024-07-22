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

	BAR_WIDTH = 48,
};

typedef struct {
	void (*fun)(void *arg);
	void *arg;
	usize line;
	u32 key;
	u32 mod;
} Hotkey;

/* action.c */
void spawn(void *arg);
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

#endif
