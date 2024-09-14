#include "c.h"
#include <windows.h>
#include <Shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <io.h>

enum {
	INPUT_BUF_SIZE = 128,
	WIDESTRING_ALLOC_BUF_SIZE = 65535, // u16 max
	WIDESTRING_MAX_STRINGS = 1024,
};

static HBRUSH background_brush;
static HBRUSH background_brush2;
static HBRUSH foreground_brush;
static u16 input[INPUT_BUF_SIZE];
static u8 input_len;
static HFONT search_font;
static HFONT options_font;
static u8 selected;

static u16 widestr_alloc[WIDESTRING_ALLOC_BUF_SIZE];
static u16 widestr_alloc_pos = 0;
static u16 widestrs[WIDESTRING_MAX_STRINGS];
static u16 widestrs_count = 0;
static u16 matching_strs[WIDESTRING_MAX_STRINGS];
static u16 matching_count = 0;
static u16 last_user_application;
static unsigned char run_as_app_launcher;

static LRESULT CALLBACK smenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static u32 isNotInvalidAscii(u16 codepoint);

static void parseStdin(void);
static void parseFolder(u16 *path, u8 apps_root_len);
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
		SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, tmp_buff);
		size_t len = wcslen(tmp_buff);
		memcpy(tmp_buff + len, L"\\*", sizeof(L"\\*"));
		parseFolder(tmp_buff, len+1);
		last_user_application = widestr_alloc_pos;
		SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, tmp_buff);
		len = wcslen(tmp_buff);
		memcpy(tmp_buff + len, L"\\*", sizeof(L"\\*"));
		parseFolder(tmp_buff, len+1);
		qsort(widestrs, widestrs_count, sizeof(widestrs[0]), sortStrings);
	} else {
		parseStdin();
	}
	memcpy(matching_strs, widestrs, widestrs_count * sizeof(widestrs[0]));
	matching_count = widestrs_count;

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
	foreground_brush  = CreateSolidBrush(DEFAULT_FOREGROUND_COLOR);

	search_font = CreateFontW(DEFAULT_SEARCH_FONT_SIZE, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, L"Tahoma");
	options_font = CreateFontW(DEFAULT_OPTIONS_FONT_SIZE, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, L"Tahoma");

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
	DeleteObject(foreground_brush);
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
			searchBoxRect.bottom = searchBoxRect.top + (DEFAULT_SEARCH_FONT_SIZE + (SEARCH_BOX_WINDOW_PADDING << 1));
			FillRect(dc, &searchBoxRect, background_brush2);

			searchBoxRect.top += SEARCH_BOX_WINDOW_PADDING;
			searchBoxRect.left += SEARCH_BOX_WINDOW_PADDING;
			SetTextColor(dc, DEFAULT_FOREGROUND_COLOR);
			SetBkMode(dc, TRANSPARENT);
			SelectObject(dc, search_font);
			DrawTextW(dc, input, input_len, &searchBoxRect, DT_LEFT | DT_TOP);

			SelectObject(dc, options_font);
			RECT optionRect = searchBoxRect;
			optionRect.top = optionRect.bottom + (SEARCH_BOX_MARGIN + SEARCH_BOX_WINDOW_PADDING); 
			optionRect.bottom = optionRect.top + DEFAULT_OPTIONS_FONT_SIZE + SEARCH_BOX_WINDOW_PADDING; 
			int option = 0;
			while (optionRect.bottom <= ps.rcPaint.bottom - MAIN_WINDOW_PADDING && option < matching_count) {
				if (option == selected) {
					SetTextColor(dc, DEFAULT_BACKGROUND_COLOR);
					RECT boxRect = optionRect;
					boxRect.top -= SEARCH_BOX_WINDOW_PADDING;
					boxRect.left -= SEARCH_BOX_WINDOW_PADDING;
					FillRect(dc, &boxRect, foreground_brush);
					DrawTextW(dc, widestr_alloc + matching_strs[option], -1, &optionRect, DT_LEFT | DT_TOP);
					SetTextColor(dc, DEFAULT_FOREGROUND_COLOR);
				} else {
					DrawTextW(dc, widestr_alloc + matching_strs[option], -1, &optionRect, DT_LEFT | DT_TOP);
				}
				optionRect.top += (DEFAULT_OPTIONS_FONT_SIZE + (SEARCH_BOX_WINDOW_PADDING << 1));
				optionRect.bottom += (DEFAULT_OPTIONS_FONT_SIZE + (SEARCH_BOX_WINDOW_PADDING << 1));
				option += 1;
			}

			EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_CHAR: {
			unsigned char check_matches = 0;
			switch (wParam) {
				case 0x1B: /* Esc */ {
					PostQuitMessage(0);
					break;
				} 
				case '\b': {
					if (input_len > 0) {
						input_len -= 1;
						input[input_len] = '\0';
						InvalidateRect(hWnd, NULL, TRUE);
					}
					memcpy(matching_strs, widestrs, widestrs_count * sizeof(widestrs[0]));
					matching_count = widestrs_count;
					check_matches = input_len != 0;
					break;
				}
				case 0x7F: /* Ctrl-\b */ {
					if (input_len > 0) {
						input_len = 0;
						input[0] = '\0';
						InvalidateRect(hWnd, NULL, TRUE);
					}
					memcpy(matching_strs, widestrs, widestrs_count * sizeof(widestrs[0]));
					matching_count = widestrs_count;
					break;
				}
				case '\r': {
					if (run_as_app_launcher) {
						u16 path[MAX_PATH];
						int folder = matching_strs[selected] < last_user_application
							? CSIDL_PROGRAMS
							: CSIDL_COMMON_PROGRAMS;
						SHGetFolderPathW(NULL, folder, NULL, SHGFP_TYPE_CURRENT, path);
						wcscat(path, L"\\");
						wcscat(path, widestr_alloc + matching_strs[selected]);
						wprintf(L"%s\n", path);
						HINSTANCE res = ShellExecuteW(NULL, L"open", path, NULL, NULL, SW_SHOWNORMAL);
						if ((int)res <= 32) {
							wprintf(L"Error: %d, %d.\n", res, ERROR_FILE_NOT_FOUND);
						}
					} else {
						fputws(widestr_alloc + matching_strs[selected], stdout);
					}
					PostQuitMessage(0);
					break;
				}
				default: {
					if (isNotInvalidAscii(wParam) && input_len < (INPUT_BUF_SIZE-1)) {
						input[input_len] = wParam;
						input_len += 1;
						input[input_len] = '\0';
						InvalidateRect(hWnd, NULL, TRUE);
						check_matches = 1;
					}
					break;
				}
			}
			if (check_matches) {
				u16 previous_matches_count = matching_count;
				u16 new_matches = 0;
				for (size_t i = 0; i < previous_matches_count; ++i) {
					if (StrStrIW(widestr_alloc + matching_strs[i], input) != NULL) {
						matching_strs[new_matches] = matching_strs[i];
						new_matches += 1;
					}
				}
				matching_count = new_matches;
			}
			return 0;
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

static void parseFolder(u16 *path, u8 apps_root_len) {
	WIN32_FIND_DATAW find_data;
	size_t len = wcslen(path);
	HANDLE find_handle = FindFirstFileW(path, &find_data);
	path[len-1] = '\0';
	wprintf(L"%s : %s\n", path, path + apps_root_len);
	if (find_handle == INVALID_HANDLE_VALUE) {
		MessageBoxA(NULL, "smenu wasn't able to open an application folder.", "smenu error", MB_ICONERROR | MB_OK);
		ExitProcess(1);
	}
	do {
		if ((FILE_ATTRIBUTE_DIRECTORY & find_data.dwFileAttributes) == 0) {
			unsigned char isDesktopIni = wcscmp(find_data.cFileName, L"desktop.ini") == 0;
			if (isDesktopIni) continue;
			widestrs[widestrs_count] = widestr_alloc_pos;
			int written = swprintf(
				widestr_alloc + widestr_alloc_pos,
				WIDESTRING_ALLOC_BUF_SIZE - widestr_alloc_pos,
				L"%s%s", path + apps_root_len, find_data.cFileName
			);
			widestr_alloc_pos += written + 1;
			widestrs_count += 1;
		} else {
			unsigned char isCurrDir = wcscmp(find_data.cFileName, L".") == 0;
			unsigned char isAboveDir = wcscmp(find_data.cFileName, L"..") == 0;
			if (isCurrDir || isAboveDir) continue;
			u16 new_dir[MAX_PATH];
			swprintf(new_dir, MAX_PATH-1, L"%s%s\\*", path, find_data.cFileName);

			parseFolder(new_dir, apps_root_len);
		}
	} while (FindNextFileW(find_handle, &find_data));
	path[len-1] = '*';
}

static int sortStrings(const void *sip1, const void *sip2) {
	u16 s1 = *(const u16 *)sip1;
	u16 s2 = *(const u16 *)sip2;
	return _wcsicmp(widestr_alloc + s1, widestr_alloc + s2);
}