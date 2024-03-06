#include "lib.h"

typedef struct {
	void (*fun)(void *arg);
	void *arg;
} Hotkeys;


/* action */
void spawn(void *arg);
void quit(void *arg);
void reloadConfig(void *arg);

/* config */
bool loadConfig(void);
void loadDefaultConfig(void);

/* main.c */
extern usize hotkeys_count;
extern Hotkeys *hotkeys;
extern usize stderr;
extern usize stdout;
extern Arena stable;
extern Arena temp;
