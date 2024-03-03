#include "lib.h"

w32(bool) UnregisterHotKey(usize handle, i32 id);

typedef struct {
	void (*fun)(void *arg);
	void *arg;
} Hotkeys;


/* cofig.c */
bool loadConfig(void);

/* main.c */
extern usize hotkeys_count;
extern Hotkeys *hotkeys;
extern usize stderr;
