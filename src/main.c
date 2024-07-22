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

static BOOL CALLBACK updateWorkArea(HMONITOR mon, HDC dc, LPRECT rect, LPARAM lparam) {
	HMONITOR main_mon = (HMONITOR) lparam;
	if (mon == main_mon) {
		rect->right -= BAR_WIDTH;
		SetWindowPos(
			bar_window,
			0,
			rect->right, rect->top, BAR_WIDTH, rect->bottom - rect->top,
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOSENDCHANGING
		);
	}
	SystemParametersInfoW(SPI_SETWORKAREA, 0, rect, 1);
	return 1;
}

usize hotkeys_count = 0;
Hotkey *hotkeys = NULL;
Hotkey hotkeys_buf[MAX_HOTKEYS];
HWND bar_window;

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
		.lpfnWndProc = barHandler
	};
	if (!RegisterClassW(&barClass)) {
		MessageBoxA(NULL, "Failed to register bar class.", "Breeze error", MB_ICONERROR | MB_OK);
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

	killProcesses();
	HMONITOR main_mon = MonitorFromPoint((POINT){0, 0}, MONITOR_DEFAULTTOPRIMARY);
	EnumDisplayMonitors(0, NULL, updateWorkArea, (LPARAM) main_mon);
	loadUserApplicationDirs();

	if (loadConfig())
	 	loadDefaultConfig();

	signal(SIGINT, pleaseshowmywindowsonctrlc);

	MSG msg;
	i32 ret;
	while ((ret = GetMessageW(&msg, 0, 0, 0)) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

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
			int x = ps.rcPaint.left;
			int y = ps.rcPaint.top;
			int w = ps.rcPaint.right - ps.rcPaint.left;
			int h = ps.rcPaint.bottom - ps.rcPaint.top;
			PatBlt(dc, x, y, w, h, WHITENESS);
			EndPaint(hWnd, &ps);
			break;
		}
		default: {
			result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
	return result;
}
