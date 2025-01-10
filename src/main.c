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


static void setProcessDpi(void);
static void killProcesses(void);
static void CALLBACK invalidateClock(HWND hWnd, u32 MSG, u64 timer, DWORD idk);
static LRESULT CALLBACK barHandler(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam);


usize hotkeys_count = 0;
Hotkey *hotkeys = NULL;
Hotkey hotkeys_buf[MAX_HOTKEYS];
HWND bar_window;
HFONT default_bar_font;
HFONT bar_font;
COLORREF background;
COLORREF foreground;
DWORD bar_font_height;
int bar_position;
int bar_pad;
int bar_width;
RECT desktop_rect;
RECT hours_rect;
RECT minutes_rect;
RECT clock_rect;
void (*drawBar)(HDC dc);


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;

	HANDLE mutex = CreateMutexA(NULL, TRUE, "breeze");
	if (mutex == NULL) {
		MessageBoxA(NULL, "Failed to initialize breeze.", "Breeze error", MB_ICONERROR | MB_OK);
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBoxA(NULL, "Breeze is already running.", "Breeze error", MB_ICONERROR | MB_OK);
		return 1;
	}
	setProcessDpi();

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

	MSG msg;
	while (GetMessageW(&msg, 0, 0, 0) > 0) {
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
		case WM_DISPLAYCHANGE: {
			reloadConfig(0);
			break;
		}
		default: {
			result = DefWindowProcW(hWnd, uMsg, wParam, lParam);
			break;
		}
	}
	return result;
}



static void setProcessDpi(void) {
	HMODULE user32_dll = LoadLibraryA("user32.dll");
	if (user32_dll == NULL) return;

	typedef BOOL (*set_process_dpi_awareness_context)(DPI_AWARENESS_CONTEXT);
	set_process_dpi_awareness_context fun1 = (set_process_dpi_awareness_context)
		GetProcAddress(user32_dll, "SetProcessDpiAwarenessContext");
	if (fun1) {
		if (fun1(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) goto freedll;
		if (fun1(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) goto freedll;
	}

	typedef BOOL (*set_process_dpi_aware)(void);
	set_process_dpi_aware fun2 = (set_process_dpi_aware)
		GetProcAddress(user32_dll, "SetProcessDPIAware");
	if (fun2) {
		fun2();
	}

freedll:
	FreeLibrary(user32_dll);
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


static void CALLBACK invalidateClock(HWND hWnd, u32 MSG, u64 timer, DWORD idk) {
	unused(hWnd);
	unused(MSG);
	unused(timer);
	unused(idk);
	InvalidateRect(bar_window, &clock_rect, FALSE);
}
