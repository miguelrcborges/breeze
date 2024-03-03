#include "lib.h"

w32(bool) UnregisterHotKey(usize handle, i32 id);

typedef struct {
	void (*fun)(void *arg);
	void *arg;
} Hotkeys;


/* action */
void spawn(void *arg);
void quit(void *arg);

/* cofig.c */
bool loadConfig(void *_);

/* main.c */
extern usize hotkeys_count;
extern Hotkeys *hotkeys;
extern usize stderr;
extern Arena stable;
extern Arena temp;
