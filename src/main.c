#include "c.h"
#include <tlhelp32.h>
#include <signal.h>

#include "action.c"
#include "config/config.c"

static const char *processesToKill[] = {
	"explorer.exe",
	"SearchApp.exe",
	"TextInputHost.exe",
	"ShellExperienceHost.exe",
};

void pleaseshowmywindowsonctrlc(int sig) {
	quit(0);
}

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


usize hotkeys_count = 0;
Hotkey *hotkeys = NULL;
Hotkey hotkeys_buf[MAX_HOTKEYS];
HWND bar_window;
HFONT default_bar_font;
HFONT bar_font;
usize bar_font_height;
COLORREF background;
COLORREF foreground;
usize bar_position;
usize bar_pad;
usize bar_width;
RECT desktop_rect;
RECT hours_rect;
RECT minutes_rect;
RECT clock_rect;
void (*drawBar)(HDC dc);

static void CALLBACK invalidateClock(HWND hWnd, u32 MSG, u64 timer, DWORD idk) {
	InvalidateRect(bar_window, &clock_rect, FALSE);
}

static LRESULT CALLBACK barHandler(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam);

#ifdef WINDOW
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;
#else
int main(void) {
	HINSTANCE hInstance = GetModuleHandle(NULL);
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

	WNDCLASSW barClass = {
		.hInstance = hInstance,
		.lpszClassName = L"breeze-bar",
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.lpfnWndProc = barHandler
	};
	if (!RegisterClassW(&barClass)) {
		MessageBoxA(NULL, "Failed to register bar class.", "Breeze error", MB_ICONERROR | MB_OK);
		return 1;
	}

	default_bar_font = CreateFontW(BAR_DEFAULT_FONT_HEIGHT, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, default_bar_font_str);
	if (default_bar_font == NULL) {
		MessageBoxA(NULL, "Faield to create a fallback font using a font already preinstalled.", "Breeze error", MB_ICONERROR | MB_OK);
		return 1;
	}

	bar_window = CreateWindowExW(
		WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
		L"breeze-bar",
		L"breeze-bar",
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, 0, 0,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	u64 timer = SetTimer(bar_window, 1, BAR_INVALIDATE_CLOCK_DURATION, invalidateClock);

	killProcesses();
	loadUserApplicationDirs();
	reloadConfig(0);

	signal(SIGINT, pleaseshowmywindowsonctrlc);

	MSG msg;
	i32 ret;
	while ((ret = GetMessageW(&msg, 0, 0, 0)) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	KillTimer(bar_window, timer);
	if (bar_font != default_bar_font) DeleteObject(bar_font);
	DeleteObject(default_bar_font);
	return 0;
}

static LRESULT CALLBACK barHandler(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (uMsg) {
		case WM_HOTKEY: {
			usize id = (usize) wParam;
			if (id < hotkeys_count) {
				hotkeys[id].fun(hotkeys[id].arg);
			}
			break;
		}
		case WM_PAINT: {
			PAINTSTRUCT ps;
			char buf[16];
			HDC dc = BeginPaint(hWnd, &ps);

			HBRUSH brush = CreateSolidBrush(background);
			FillRect(dc, &(ps.rcPaint), brush);
			DeleteObject(brush);

			SetBkMode(dc, TRANSPARENT);
			SelectObject(dc, bar_font);
			SetTextColor(dc, foreground);

			drawBar(dc);

			EndPaint(hWnd, &ps);
			break;
		}
		default: {
			result = DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
	}
	return result;
}
