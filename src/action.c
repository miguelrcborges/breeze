#include "c.h"

typedef struct {
	u32 size;
	u16 *reserved;
	u16 *desktop;
	u16 *title;
	u32 x;
	u32 y;
	u32 width;
	u32 height;
	u32 columns;
	u32 rows;
	u32 fill;
	u32 flags;
	u16 show;
	u16 reserved2;
	void *reserved3;
	usize stdin;
	usize stdout;
	usize stderr;
} StartupInfo;

typedef struct {
	usize process;
	usize thread;
	usize PID;
	usize threadID;
} ProcessInformation;

w32(i32) CreateProcessW(u16 *exe, u16 *cmdline, void *pAttr, void *tAttr, u32 inherit, u32 flags, void *env, char *dir, StartupInfo *startupInfo, void *procInfo);
w32(i32) CloseHandle(usize handle);
w32(void) ExitProcess(u32 code);

void spawn(void *arg) {
	io_write(stderr, string("[SPAWN] Called\n"));
	u16 *command = arg;

	StartupInfo si = {
		.size = sizeof(StartupInfo)
	};
	ProcessInformation pi;

	CreateProcessW(0, command, 0, 0, 0, 0, 0, 0, &si, &pi);
	CloseHandle(pi.thread);
	CloseHandle(pi.process);
}

void quit(void *arg) {
	u32 code = (u32)(usize) arg;
	u16 explorer[] = L"explorer.exe";
	spawn(explorer);
	ExitProcess(code);
}

void reloadConfig(void *arg) {
	Arena_free(&stable);
	if (loadConfig())
		loadDefaultConfig();
}
