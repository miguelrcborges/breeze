#include "c.h"

static HWND windows[MAX_DESKTOPS][MAX_WINDOWS_PER_DESKTOP];
static usize windows_count[MAX_DESKTOPS];
static usize current_desktop;

void spawn(void *arg) {
	u16 *command = arg;

	STARTUPINFOW si = {
		.cb = sizeof(STARTUPINFOW)
	};
	PROCESS_INFORMATION pi;

	CreateProcessW(0, command, 0, 0, 0, CREATE_NEW_PROCESS_GROUP, 0, 0, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void quit(void *arg) {
	u32 code = (u32)(usize) arg;
	u16 explorer[] = L"explorer.exe";
	spawn(explorer);
	revealAllWindows(NULL);
	ExitProcess(code);
}

void reloadConfig(void *arg) {
	if (loadConfig())
		loadDefaultConfig();
}

void kill(void *arg) {
	PostMessageA(GetForegroundWindow(), WM_CLOSE, 0, 0);
}


static BOOL CALLBACK visitWindow(HWND w, LPARAM _) {
	if (IsWindowVisible(w)) {
		windows[current_desktop][windows_count[current_desktop]++] = w;
		ShowWindow(w, SW_HIDE);
	}
	EnumChildWindows(w, visitWindow, 0);
	return TRUE;
}

void switchToDesktop(void *t_desktop) {
	usize desktop = (usize) t_desktop;
	if (desktop == current_desktop) return;
	windows_count[current_desktop] = 0;
	EnumWindows(visitWindow, 0);
	for (size_t i = 0; i < windows_count[desktop]; ++i) {
		if (IsWindow(windows[desktop][i])) {
			ShowWindow(windows[desktop][i], SW_SHOW);
		}
	}
	current_desktop = desktop;
}

void sendToDesktop(void *t_desktop) {
	usize desktop = (usize) t_desktop;
	HWND w = GetForegroundWindow();
	if (w == NULL) return;
	ShowWindow(w, SW_HIDE);
	windows[desktop][windows_count[desktop]++] = w;
}

void revealAllWindows(void *_) {
	for (usize i = 0; i < MAX_DESKTOPS; ++i) {
		if (i == current_desktop) continue;
		for (usize ii = 0; ii < windows_count[i]; ++ii) {
			if (IsWindow(windows[i][ii])) {
				ShowWindow(windows[i][ii], SW_SHOW);
			}
		}
		windows_count[i] = 0;
	}
}
