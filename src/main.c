#include "c.h"

typedef struct {
  u32 size;
  u32 countUsage;
  u32 PID;
  u32 *defaultHeapID;
  u32 moduleID;
  u32 countThreads;
  u32 parentPID;
  u32 pcPriClassBase;
  u32 flags;
  u8 exeFile[260];
} ProcessEntry32;

typedef struct {
	usize handle;
	u32 message;
	void *param1;
	uptr param2;
	u32 time;
	struct {
		long x;
		long y;
	} Point;
	u32 private;
} Message;

w32(usize) CreateToolhelp32Snapshot(u64 flags, u64 PID);
w32(bool) Process32First(u64 snapshot, ProcessEntry32 *p);
w32(bool) Process32Next(u64 snapshot, ProcessEntry32 *p);
w32(bool) Process32Next(u64 snapshot, ProcessEntry32 *p);
w32(i32) CloseHandle(usize handle);
w32(usize) OpenProcess(u64 access, bool inherit, u64 PID);
w32(i32) TerminateProcess(usize handle, u32 code);
w32(i32) SystemParametersInfoW(u32 action, u32 param, void *vparam, u32 ini);
w32(i32) EnumDisplayMonitors(usize handle, void *rect, i32 (*enumProc)(usize mon, usize param1, void *rect, void *lparam), u32 data);
w32(i32) RegisterHotKey(usize handle, i32 id, u32 mod, u32 keycode);
w32(i32) GetMessageW(Message *msg, usize handle, u32 filterMin, u32 filterMax);
w32(void) ExitProcess(u32 code);

static const char *processesToKill[] = {
	"explorer.exe",
	"SearchApp.exe",
	"TextInputHost.exe",
};

static bool u8buf_areEqual(const u8 *restrict f, const u8 *restrict s) {
	while (*f != '\0' && *s != '\0') {
		if (*f == *s) {
			f += 1;
			s += 1;
		} else {
			return 0;
		}
	}
	return *f == '\0' && *s == '\0';
}

static void killProcesses(void) {
	usize snapshot = CreateToolhelp32Snapshot(2, 0);

	if (snapshot) {
		ProcessEntry32 pe32;
		pe32.size = sizeof(ProcessEntry32);

		if (Process32First(snapshot, &pe32)) {
			do {
				for (usize p = 0; p < len(processesToKill); ++p) {
					if (u8buf_areEqual(pe32.exeFile, (u8 *) processesToKill[p])) {
						usize process = OpenProcess(1, 0, pe32.PID);
						if (process != 0) {
							TerminateProcess(process, 1); 
							CloseHandle(process);
							break;
						}
					}
				}
			} while (Process32Next(snapshot, &pe32));
		}
		CloseHandle(snapshot);
	}
}

static i32 __stdcall updateWorkArea(usize mon, usize param1, void *rect, void *lparam) {
	SystemParametersInfoW(0x2F, 0, rect, 1);
	return 1;
}

usize stderr;
usize stdout;
usize hotkeys_count = 0;
Hotkeys *hotkeys = NULL;
Arena stable;
Arena temp;

int mainCRTStartup(void) {
	stderr = getStdErr();
	stdout = getStdOut();
	stable = Arena_create(0);
	temp = Arena_create(0);
	// if (loadConfig())
	// 	loadDefaultConfig();

	killProcesses();
	EnumDisplayMonitors(0, NULL, updateWorkArea, 0);

	Message msg;
	i32 ret;
	while ((ret = GetMessageW(&msg, 0, 0, 0))) {
		if (ret == -1) {
			ExitProcess(-1);
		}

		switch (msg.message) {
			case 0x0312: {
				usize id = (u64) msg.param1;
				if (id <= hotkeys_count) {
					hotkeys[id].fun(hotkeys[id].arg);
				}
			}
		}
	}

	return 0;
}
