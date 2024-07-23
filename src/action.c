#include "c.h"

static HWND focused_window[MAX_DESKTOPS];
static HWND windows[MAX_DESKTOPS][MAX_WINDOWS_PER_DESKTOP];
static usize windows_count[MAX_DESKTOPS];
static usize current_desktop = 1;

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
	if (w == bar_window) return TRUE;
	if (IsWindowVisible(w)) {
		windows[current_desktop][windows_count[current_desktop]++] = w;
		ShowWindow(w, SW_HIDE);
	}
	return TRUE;
}

void switchToDesktop(void *t_desktop) {
	usize desktop = (usize) t_desktop;
	if (desktop == current_desktop) return;
	focused_window[current_desktop] = GetForegroundWindow();
	windows_count[current_desktop] = 0;
	EnumWindows(visitWindow, 0);
	for (size_t i = 0; i < windows_count[desktop]; ++i) {
		if (IsWindow(windows[desktop][i])) {
			ShowWindow(windows[desktop][i], SW_SHOW);
		}
	}
	if (IsWindow(focused_window[desktop])) {
		SetForegroundWindow(focused_window[desktop]);
	}
	current_desktop = desktop;
	InvalidateRect(bar_window, NULL, TRUE);
}

void sendToDesktop(void *t_desktop) {
	usize desktop = (usize) t_desktop;
	if (desktop == current_desktop) return;
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

void focusNext(void *_) {
	HWND w = GetForegroundWindow();
	w = GetWindow(w, GW_HWNDLAST);
	while (1) {
		if (unlikely(w == NULL)) return;
		u32 styles = GetWindowLongW(w, GWL_STYLE);
		u32 ex_styles = GetWindowLongW(w, GWL_EXSTYLE);
		if ((styles & WS_VISIBLE) && ((ex_styles & WS_EX_NOACTIVATE) ^ WS_EX_NOACTIVATE)) break;
		w = GetWindow(w, GW_HWNDPREV);
	}
	SetForegroundWindow(w);
}

void focusPrev(void *_) {
	HWND w = GetForegroundWindow();
	do {
		w = GetWindow(w, GW_HWNDNEXT);
		if (unlikely(w == NULL)) return;
	} while (!IsWindowVisible(w));
	SetForegroundWindow(w);
}
