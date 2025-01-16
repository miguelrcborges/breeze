#include "c.h"
#include <windows.h>
#include <stdio.h>

static HWND focused_window[MAX_DESKTOPS];
static HWND windows[MAX_DESKTOPS][MAX_WINDOWS_PER_DESKTOP];
static usize windows_count[MAX_DESKTOPS];

typedef struct {
	BreezeState *state;
	HMONITOR main_monitor;
} UpdateWorkAreaLPARAM;

static BOOL CALLBACK updateWorkArea(HMONITOR mon, HDC dc, LPRECT rect, LPARAM lparam) {
	unused(dc);

	UpdateWorkAreaLPARAM *args = (UpdateWorkAreaLPARAM *)lparam;
	if (mon == args->main_monitor) {
		int x, y, w, h;
		switch (args->state->bar.position) {
			case BAR_LEFT: {
				x = rect->left;
				y = rect->top;
				w = args->state->bar.width;
				h = rect->bottom - rect->top;
				rect->left += args->state->bar.width;
				args->state->bar.draw_function = drawVertical24hClock;
				break;
			};
			case BAR_TOP: {
				x = rect->left;
				y = rect->top;
				w = rect->right - rect->left;
				h = args->state->bar.width;
				rect->top += args->state->bar.width;
				args->state->bar.draw_function = drawHorizontal24hClock;
				break;
			};
			case BAR_RIGHT: {
				x = rect->right - args->state->bar.width;
				y = rect->top;
				w = args->state->bar.width;
				h = rect->bottom - rect->top;
				rect->right -= args->state->bar.width;
				args->state->bar.draw_function = drawVertical24hClock;
				break;
			};
			case BAR_BOTTOM: {
				x = rect->left;
				y = rect->bottom - args->state->bar.width;
				w = rect->right - rect->left;
				h = args->state->bar.width;
				rect->bottom -= args->state->bar.width;
				args->state->bar.draw_function = drawHorizontal24hClock;
				break;
			};
			default: {
				MessageBoxA(NULL, "Invalid condition found. Exiting.", "Breeze Adding Hotkey Error", MB_OK | MB_ICONWARNING);
				quit(args->state, (void*)1);
				__builtin_unreachable();
			}
		}
		SetWindowPos(
			args->state->bar.window,
			0,
			x, y, w, h,
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOSENDCHANGING
		);
	}
	SystemParametersInfoW(SPI_SETWORKAREA, 0, rect, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
	return 1;
}

void spawn(BreezeState *state, void *arg) {
	unused(state);

	u16 *command = (u16 *)arg;

	STARTUPINFOW si = {
		.cb = sizeof(STARTUPINFOW)
	};
	PROCESS_INFORMATION pi;

	CreateProcessW(0, command, 0, 0, 0, CREATE_NEW_PROCESS_GROUP | CREATE_NO_WINDOW, 0, 0, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void spawnWithoutConsole(BreezeState *state, void *arg) {
	unused(state);

	u16 *command = arg;

	STARTUPINFOW si = {
		.cb = sizeof(STARTUPINFOW)
	};
	PROCESS_INFORMATION pi;

	CreateProcessW(0, command, 0, 0, 0, CREATE_NEW_PROCESS_GROUP, 0, 0, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void quit(BreezeState *s, void *arg) {
	u32 code = (u32)(usize) arg;
	u16 explorer[] = L"explorer.exe";
	spawn(s, explorer);
	revealAllWindows(s, NULL);
	ExitProcess(code);
}

void reloadConfig(BreezeState *state, void *arg) {
	unused(arg);
	loadConfig(state);

	HMONITOR main_mon = MonitorFromPoint((POINT){0, 0}, MONITOR_DEFAULTTOPRIMARY);
	UpdateWorkAreaLPARAM params = {
		.main_monitor = main_mon,
		.state = state
	};
	EnumDisplayMonitors(0, NULL, updateWorkArea, (LPARAM) &params);
	InvalidateRect(state->bar.window, NULL, TRUE);
}

void kill(BreezeState *s, void *arg) {
	unused(s);
	unused(arg);

	PostMessageA(GetForegroundWindow(), WM_CLOSE, 0, 0);
}

static BOOL CALLBACK visitWindow(HWND w, LPARAM _breezeState) {
	BreezeState *breezeState = (BreezeState *)_breezeState;
	if (w == breezeState->bar.window) return TRUE;
	if (IsWindowVisible(w)) {
		for (usize d = 0; d < MAX_DESKTOPS; ++d) {
			if (d == breezeState->current_desktop) continue;
			for (usize i = 0; i < windows_count[d]; ++i) {
				if (w == windows[d][i]) {
					windows[d][i] = INVALID_HANDLE_VALUE;
				}
			}
		}
		windows[breezeState->current_desktop][windows_count[breezeState->current_desktop]] = w;
		windows_count[breezeState->current_desktop] += 1;
		ShowWindow(w, SW_HIDE);
	} else {
		EnumChildWindows(w, visitWindow, _breezeState);
	}
	return TRUE;
}

void switchToDesktop(BreezeState *s, void *t_desktop) {
	usize desktop = (usize) t_desktop;
	if (desktop == s->current_desktop) return;
	focused_window[s->current_desktop] = GetForegroundWindow();
	windows_count[s->current_desktop] = 0;
	EnumWindows(visitWindow, (LPARAM)s);
	for (size_t i = 0; i < windows_count[desktop]; ++i) {
		if (IsWindow(windows[desktop][i])) {
			ShowWindow(windows[desktop][i], SW_SHOW);
		}
	}
	if (IsWindow(focused_window[desktop])) {
		SetForegroundWindow(focused_window[desktop]);
	}
	s->current_desktop = desktop;
	InvalidateRect(s->bar.window, NULL, FALSE);
}

void sendToDesktop(BreezeState *s, void *t_desktop) {
	unused(s);

	usize desktop = (usize) t_desktop;
	if (desktop == s->current_desktop) return;
	HWND w = GetForegroundWindow();
	if (w == NULL) return;
	ShowWindow(w, SW_HIDE);
	windows[desktop][windows_count[desktop]] = w;
	windows_count[desktop] += 1;
}

void revealAllWindows(BreezeState *s, void *__unused) {
	unused(__unused);

	for (usize i = 0; i < MAX_DESKTOPS; ++i) {
		if (i == s->current_desktop) continue;
		for (usize ii = 0; ii < windows_count[i]; ++ii) {
			if (IsWindow(windows[i][ii])) {
				ShowWindow(windows[i][ii], SW_SHOW);
			}
		}
		windows_count[i] = 0;
	}
}

void focusNext(BreezeState *s, void *__unused) {
	unused(__unused);
	unused(s);

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

void focusPrev(BreezeState *s, void *__unused) {
	unused(__unused);
	unused(s);

	HWND w = GetForegroundWindow();
	do {
		w = GetWindow(w, GW_HWNDNEXT);
		if (unlikely(w == NULL)) return;
	} while (!IsWindowVisible(w));
	SetForegroundWindow(w);
}
