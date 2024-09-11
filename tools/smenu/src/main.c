#include "c.h"
#include <windows.h>
#include <Shlobj.h>
#include <stdio.h>
#include <io.h>

enum {
	INPUT_BUF_SIZE = 128,
	WIDESTRING_ALLOC_BUF_SIZE = 65535, // u16 max
	WIDESTRING_MAX_STRINGS = 1024,
};

static HBRUSH background_brush;
static HBRUSH background_brush2;
static u16 input[INPUT_BUF_SIZE];
static u8 input_len;
static HFONT ui_font;

static u16 widestr_alloc[WIDESTRING_ALLOC_BUF_SIZE];
static u16 widestr_alloc_pos = 0;
static u16 widestrs[WIDESTRING_MAX_STRINGS];
static u16 widestrs_count = 0;
static u16 last_local_user_application;
static unsigned char run_as_app_launcher;

static LRESULT CALLBACK smenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static u32 isNotInvalidAscii(u16 codepoint);

static void parseStdin(void);
static void parseFolder(u16 *path);
static int sortStrings(const void *sip1, const void *sip2);

int main(void) {
	HINSTANCE hInstance = GetModuleHandleW(NULL);
	HANDLE mutex = CreateMutexA(NULL, TRUE, "smenu");
	if (mutex == NULL) {
		MessageBoxA(NULL, "Failed to initialize smenu.", "smenu error", MB_ICONERROR | MB_OK);
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBoxA(NULL, "smenu is already running.", "smenu error", MB_ICONERROR | MB_OK);
		return 1;
	}

	WNDCLASSW smenu_class = {
		.hInstance = hInstance,
		.lpszClassName = L"smenu",
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.lpfnWndProc = smenuProc,
	};
	if (!RegisterClassW(&smenu_class)) {
		MessageBoxA(NULL, "Failed to register smenu class.", "smenu error", MB_ICONERROR | MB_OK);
		return 1;
	}

	run_as_app_launcher = _isatty(_fileno(stdin)) ? 1u : 0u;
	if (run_as_app_launcher) {
		u16 tmp_buff[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_STARTMENU, NULL, SHGFP_TYPE_CURRENT, tmp_buff);
		wcscat(tmp_buff, L"\\Programs\\*");
		parseFolder(tmp_buff);
		last_local_user_application = widestrs_count;
		SHGetFolderPathW(NULL, CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_CURRENT, tmp_buff);
		wcscat(tmp_buff, L"\\Programs\\*");
		parseFolder(tmp_buff);
	} else {
		parseStdin();
	}
	qsort(widestrs, widestrs_count, sizeof(widestrs[0]), sortStrings);

	for (size_t i = 0; i < widestrs_count; i += 1) {
		wprintf(L"%s\n", widestr_alloc + widestrs[i]);
	}

	POINT cursor_pos;
	GetCursorPos(&cursor_pos);
	HMONITOR current_mon = MonitorFromPoint(cursor_pos, MONITOR_DEFAULTTONEAREST);
	MONITORINFO current_mon_info = {
		.cbSize = sizeof(current_mon_info),
	};
	GetMonitorInfoA(current_mon, (MONITORINFO *)&current_mon_info);

	int mon_width = current_mon_info.rcMonitor.right - current_mon_info.rcMonitor.left;
	int mon_height = current_mon_info.rcMonitor.bottom - current_mon_info.rcMonitor.top;
	int win_width = mon_width >> 1;
	int win_height = mon_height >> 1;
	int win_left = (win_width >> 1) + current_mon_info.rcMonitor.left;
	int win_top = (win_height >> 1) + current_mon_info.rcMonitor.top;
	background_brush = CreateSolidBrush(DEFAULT_BACKGROUND_COLOR);
	background_brush2 = CreateSolidBrush(DEFAULT_BACKGROUND_COLOR2);

	ui_font = CreateFontW(DEFAULT_FONT_SIZE, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, L"Tahoma");

	HWND parent = CreateWindowExW(
		WS_EX_TOPMOST,
		L"smenu",
		L"smenu",
		WS_POPUP | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		win_left,
		win_top,
		win_width,
		win_height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	MSG msg;
	while (GetMessageW(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	ReleaseMutex(mutex);
	DeleteObject(background_brush);
	DeleteObject(background_brush2);
	return 0;
}


static LRESULT CALLBACK smenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (uMsg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hWnd, &ps);

			FillRect(dc, &(ps.rcPaint), background_brush);

			RECT searchBoxRect = ps.rcPaint;
			searchBoxRect.top += MAIN_WINDOW_PADDING;
			searchBoxRect.left += MAIN_WINDOW_PADDING;
			searchBoxRect.right -= MAIN_WINDOW_PADDING;
			searchBoxRect.bottom = searchBoxRect.top + (DEFAULT_FONT_SIZE + (SEARCH_BOX_WINDOW_PADDING << 1));
			FillRect(dc, &searchBoxRect, background_brush2);

			searchBoxRect.top += SEARCH_BOX_WINDOW_PADDING;
			searchBoxRect.left += SEARCH_BOX_WINDOW_PADDING;
			SetTextColor(dc, DEFAULT_FOREGROUND_COLOR);
			SetBkMode(dc, TRANSPARENT);
			SelectObject(dc, ui_font);
			DrawTextW(dc, input, input_len, &searchBoxRect, DT_LEFT | DT_TOP);
			EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_CHAR: {
			switch (wParam) {
				case 0x1B: {
					PostQuitMessage(0);
					return 0;
				} 
				case '\b': {
					if (input_len > 0) {
						input_len -= 1;
						InvalidateRect(hWnd, NULL, TRUE);
					}
					return 0;
				}
				default: {
					if (isNotInvalidAscii(wParam) && input_len < INPUT_BUF_SIZE) {
						input[input_len] = wParam;
						input_len += 1;
						InvalidateRect(hWnd, NULL, TRUE);
					}
					return 0;
				}
			}
		}
		case WM_KILLFOCUS: {
			PostQuitMessage(0);
			return 0;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		default: {
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
	}
}


static u32 isNotInvalidAscii(u16 codepoint) {
	return codepoint >= ' ';
}

static void parseStdin(void) {
	u16 read_buf[1024];
	while (fgetws(read_buf, 1024, stdin)) {
		if (read_buf[0] == '\n') continue;
		widestrs[widestrs_count] = widestr_alloc_pos;
		for (size_t i = 0; read_buf[i] != '\n' && read_buf[i] != '\0'; i += 1) {
			widestr_alloc[widestr_alloc_pos] = read_buf[i];
			widestr_alloc_pos += 1;
		}
		widestr_alloc[widestr_alloc_pos] = '\0';
		widestr_alloc_pos += 1;
		widestrs_count += 1;
	}
}

static void parseFolder(u16 *path) {
	WIN32_FIND_DATAW find_data;
	size_t len = wcslen(path);
	HANDLE find_handle = FindFirstFileW(path, &find_data);
	path[len-1] = '\0';
	if (find_handle == INVALID_HANDLE_VALUE) {
		MessageBoxA(NULL, "smenu wasn't able to open an application folder.", "smenu error", MB_ICONERROR | MB_OK);
		ExitProcess(1);
	}
	do {
		if ((FILE_ATTRIBUTE_DIRECTORY & find_data.dwFileAttributes) == 0) {
			unsigned char isDesktopIni = wcscmp(find_data.cFileName, L"desktop.ini") == 0;
			if (isDesktopIni) continue;
			widestrs[widestrs_count] = widestr_alloc_pos;
			for (size_t i = 0; find_data.cFileName[i] != '\0'; i += 1) {
				widestr_alloc[widestr_alloc_pos] = find_data.cFileName[i];
				widestr_alloc_pos += 1;
			}
			widestr_alloc[widestr_alloc_pos] = '\0';
			widestr_alloc_pos += 1;
			widestrs_count += 1;
		} else {
			unsigned char isCurrDir = wcscmp(find_data.cFileName, L".") == 0;
			unsigned char isAboveDir = wcscmp(find_data.cFileName, L"..") == 0;
			if (isCurrDir || isAboveDir) continue;
			u16 new_dir[MAX_PATH];
			swprintf(new_dir, MAX_PATH-1, L"%s%s\\*", path, find_data.cFileName);
			parseFolder(new_dir);
		}
	} while (FindNextFileW(find_handle, &find_data));
	path[len-1] = '*';
}

static int sortStrings(const void *sip1, const void *sip2) {
	u16 s1 = *(const u16 *)sip1;
	u16 s2 = *(const u16 *)sip2;
	return _wcsicmp(widestr_alloc + s1, widestr_alloc + s2);
}
