#include "c.h"
#include "tlhelp32.h"

#include "action.c"
#include "config/config.c"

static const char *processesToKill[] = {
	"explorer.exe",
	"SearchApp.exe",
	"TextInputHost.exe",
};

static void killProcesses(void) {
	HANDLE snapshot = CreateToolhelp32Snapshot(2, 0);

	if (snapshot) {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(snapshot, &pe32)) {
			do {
				for (usize p = 0; p < len(processesToKill); ++p) {
					if (strcmp(pe32.szExeFile, processesToKill[p]) == 0) {
						HANDLE process = OpenProcess(1, 0, pe32.th32ProcessID);
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

static BOOL __stdcall updateWorkArea(HMONITOR mon, HDC param1, LPRECT rect, LPARAM lparam) {
	SystemParametersInfoW(0x2F, 0, rect, 1);
	return 1;
}

usize hotkeys_count = 0;
Hotkey *hotkeys = NULL;
Hotkey hotkeys_buf[MAX_HOTKEYS];

#ifdef WINDOW
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;
#else
int main(void) {
#endif
	HANDLE mutex = CreateMutexA(NULL, TRUE, "breeze");
	if (mutex == NULL) {
		MessageBoxA(NULL, "Failed to initialize breeze.", "Breeze error", MB_ICONERROR | MB_OK);
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBoxA(NULL, "Breeze is already running.", "Breeze error", MB_ICONERROR | MB_OK);
		return 1;
	}

	killProcesses();
	EnumDisplayMonitors(0, NULL, updateWorkArea, 0);
	loadUserApplicationDirs();

	if (loadConfig())
	 	loadDefaultConfig();

	MSG msg;
	i32 ret;
	while ((ret = GetMessageW(&msg, 0, 0, 0))) {
		if (ret == -1) {
			quit(0);
		}

//		switch (msg.message) {
//			case WM_HOTKEY: {
				usize id = (usize) msg.wParam;
				if (id <= hotkeys_count) {
					hotkeys[id].fun(hotkeys[id].arg);
				}
//			}
//		}
	}

	return 0;
}
