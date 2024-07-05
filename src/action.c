#include "c.h"

void spawn(void *arg) {
	u16 *command = arg;

	STARTUPINFOW si = {
		.cb = sizeof(STARTUPINFOW)
	};
	PROCESS_INFORMATION pi;

	CreateProcessW(0, command, 0, 0, 0, 0, 0, 0, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void quit(void *arg) {
	u32 code = (u32)(usize) arg;
	u16 explorer[] = L"explorer.exe";
	spawn(explorer);
	ExitProcess(code);
}

void reloadConfig(void *arg) {
	if (loadConfig())
		loadDefaultConfig();
}

void kill(void *arg) {
	PostMessageA(GetForegroundWindow(), WM_CLOSE, 0, 0);
}
