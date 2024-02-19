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

static void killProcess(const char *exeName) {
	const u8 *exe = (u8 *)exeName;
	usize snapshot = CreateToolhelp32Snapshot(2, 0);

	if (snapshot) {
		ProcessEntry32 pe32;
		pe32.size = sizeof(ProcessEntry32);

		if (Process32First(snapshot, &pe32)) {
			do {
				if (u8buf_areEqual(pe32.exeFile, exe)) {
					usize process = OpenProcess(1, 0, pe32.PID);
					if (process != 0) {
						TerminateProcess(process, 2); 
						CloseHandle(process);
					}
				}
			} while (Process32Next(snapshot, &pe32));
		}
		CloseHandle(snapshot);
	}
}

int main(void) {
	killProcess("explorer.exe");
	killProcess("SearchApp.exe");
}
