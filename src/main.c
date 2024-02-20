#include "lib.h"

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

w32(usize) CreateToolhelp32Snapshot(u64 flags, u64 PID);
w32(bool) Process32First(u64 snapshot, ProcessEntry32 *p);
w32(bool) Process32Next(u64 snapshot, ProcessEntry32 *p);
w32(bool) Process32Next(u64 snapshot, ProcessEntry32 *p);
w32(bool) CloseHandle(usize handle);
w32(usize) OpenProcess(u64 access, bool inherit, u64 PID);
w32(bool) TerminateProcess(usize handle, u32 code);
w32(bool) SystemParametersInfoW(u32 action, u32 param, void *vparam, u32 ini);
w32(bool) EnumDisplayMonitors(usize handle, void *rect, bool (*enumProc)(usize mon, usize param1, void *rect, void *lparam), u32 data);

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

static bool __stdcall updateWorkArea(usize mon, usize param1, void *rect, void *lparam) {
	SystemParametersInfoW(0x2F, 0, rect, 1);
	return 1;
}

int mainCRTStartup(void) {
	killProcesses();
	EnumDisplayMonitors(0, NULL, updateWorkArea, 0);
	return 0;
}
