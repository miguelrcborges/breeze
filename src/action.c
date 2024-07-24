#include "c.h"
#include <windows.h>
#include <stdio.h>

static HWND focused_window[MAX_DESKTOPS];
static HWND windows[MAX_DESKTOPS][MAX_WINDOWS_PER_DESKTOP];
static usize windows_count[MAX_DESKTOPS];
static usize current_desktop = 1;

static void drawVertical24hClock(HDC dc);
static void drawHorizontal24hClock(HDC dc);

static BOOL CALLBACK updateWorkArea(HMONITOR mon, HDC dc, LPRECT rect, LPARAM lparam) {
	HMONITOR main_mon = (HMONITOR) lparam;
	if (mon == main_mon) {
		int x, y, w, h;
		switch (bar_position) {
			case BAR_LEFT: {
				x = rect->left;
				y = rect->top;
				w = bar_width;
				h = rect->bottom - rect->top;
				rect->left += bar_width;

				desktop_rect.left = 0;
				desktop_rect.right = w;
				hours_rect.left = 0;
				hours_rect.right = w;
				minutes_rect.left = 0;
				minutes_rect.right = w;
				clock_rect.left = 0;
				clock_rect.right = w;

				desktop_rect.top = bar_pad;
				desktop_rect.bottom = desktop_rect.top + bar_font_height;

				minutes_rect.bottom = h - bar_pad;
				minutes_rect.top = minutes_rect.bottom - bar_font_height;

				hours_rect.bottom = minutes_rect.top;
				hours_rect.top = hours_rect.bottom - bar_font_height;

				clock_rect.top = hours_rect.top;
				clock_rect.bottom = minutes_rect.bottom;
				drawBar = drawVertical24hClock;
				break;
			};
			case BAR_TOP: {
				x = rect->left;
				y = rect->top;
				w = rect->right - rect->left;
				h = bar_width;
				rect->top += bar_width;

				desktop_rect.top = 0;
				desktop_rect.bottom = h;
				clock_rect.top = 0;
				clock_rect.bottom = h;

				desktop_rect.left = bar_pad;
				desktop_rect.right = w;

				clock_rect.right = w - bar_pad;
				clock_rect.left = 0;
				drawBar = drawHorizontal24hClock;
				break;
			};
			case BAR_RIGHT: {
				rect->right -= bar_width;
				x = rect->right;
				y = rect->top;
				w = bar_width;
				h = rect->bottom - rect->top;

				desktop_rect.left = 0;
				desktop_rect.right = w;
				hours_rect.left = 0;
				hours_rect.right = w;
				minutes_rect.left = 0;
				minutes_rect.right = w;

				desktop_rect.top = bar_pad;
				desktop_rect.bottom = desktop_rect.top + bar_font_height;

				minutes_rect.bottom = h - bar_pad;
				minutes_rect.top = minutes_rect.bottom - bar_font_height;

				hours_rect.bottom = minutes_rect.top;
				hours_rect.top = hours_rect.bottom - bar_font_height;

				clock_rect.top = hours_rect.top;
				clock_rect.bottom = minutes_rect.bottom;
				drawBar = drawVertical24hClock;
				break;
			};
			case BAR_BOTTOM: {
				rect->bottom -= bar_width;
				x = rect->left;
				y = rect->bottom;
				w = rect->right - rect->left;
				h = bar_width;

				desktop_rect.top = 0;
				desktop_rect.bottom = h;
				clock_rect.top = 0;
				clock_rect.bottom = h;

				desktop_rect.left = bar_pad;
				desktop_rect.right = w;

				clock_rect.right = w - bar_pad;
				clock_rect.left = 0;
				drawBar = drawHorizontal24hClock;
				break;
			};
		}
		SetWindowPos(
			bar_window,
			0,
			x, y, w, h,
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOSENDCHANGING
		);
	}
	SystemParametersInfoW(SPI_SETWORKAREA, 0, rect, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
	return 1;
}

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

	HMONITOR main_mon = MonitorFromPoint((POINT){0, 0}, MONITOR_DEFAULTTOPRIMARY);
	EnumDisplayMonitors(0, NULL, updateWorkArea, (LPARAM) main_mon);
	InvalidateRect(bar_window, NULL, TRUE);
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
	InvalidateRect(bar_window, &desktop_rect, FALSE);
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



// drawing clocks lol
static void drawVertical24hClock(HDC dc) {
	char buf[16];
	sprintf(buf, "%zu", current_desktop);
	DrawTextA(dc, buf, -1, &desktop_rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);

	SYSTEMTIME lt;
	GetLocalTime(&lt);
	sprintf(buf, "%02hu", lt.wHour);
	DrawTextA(dc, buf, -1, &hours_rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
	sprintf(buf, "%02hu", lt.wMinute);
	DrawTextA(dc, buf, -1, &minutes_rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
}


static void drawHorizontal24hClock(HDC dc) {
	char buf[16];
	sprintf(buf, "%zu", current_desktop);
	DrawTextA(dc, buf, -1, &desktop_rect, DT_VCENTER | DT_LEFT | DT_SINGLELINE);

	SYSTEMTIME lt;
	GetLocalTime(&lt);
	sprintf(buf, "%02hu:%02hu", lt.wHour, lt.wMinute);
	DrawTextA(dc, buf, -1, &clock_rect, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);
}
