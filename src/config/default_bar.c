#include "c.h"

static BreezePluginCleanupFunction(DefaultBarCleanup);
static BreezePluginWorkspaceChangeCallback(DefaultBarWorkspaceCallback);
static LRESULT CALLBACK DefaultBreezeBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BreezePluginWorkspaceChangeCallback *previousWorkspaceCallback;

static HFONT font;
static COLORREF foreground_color;
static COLORREF background_color;
static i32 padding;

static HWND bar_window;
static WNDCLASSW bar_window_class;
static HBRUSH background_brush;

static SYSTEMTIME system_time;
static i8 current_workspace;


static BreezePluginSetupFunction(SetupDefaultBar) {
	u16 *font_name = L"Tahoma";
	i32 font_height = 16;
	i32 bar_thickness = 32;
	foreground_color = 0xefe4e4;
	padding = 24;

	i32 main_mon_dpi = state->Monitors.buffer[state->Monitors.main_monitor_index].dpi;
#define Scale(x) x = x * main_mon_dpi / 96;
	Scale(font_height);
	Scale(bar_thickness);
	Scale(padding);
#undef Scale

	font = CreateFontW(font_height, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, font_name);

	state->Plugins.cleanup_function[state->Plugins.count] = DefaultBarCleanup;
	previousWorkspaceCallback = state->Plugins.workspace_change_callback;
	state->Plugins.workspace_change_callback = DefaultBarWorkspaceCallback;
	current_workspace = state->current_workspace;

	RECT *main_monitor_rect = &state->Monitors.buffer[state->Monitors.main_monitor_index].monitor_area;
	i32 bar_x = main_monitor_rect->left;
	i32 bar_y = main_monitor_rect->top;
	i32 bar_width = main_monitor_rect->right - main_monitor_rect->left;
	main_monitor_rect->top += bar_thickness;
	SystemParametersInfoW(SPI_SETWORKAREA, 0, main_monitor_rect, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);

	bar_window_class.hInstance = state->hInstance;
	bar_window_class.lpszClassName = L"DefaultBreezeBar";
	bar_window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	bar_window_class.lpfnWndProc = DefaultBreezeBarProc;
	RegisterClassW(&bar_window_class);

	background_brush = CreateSolidBrush(background_color);

	bar_window = CreateWindowExW(
		WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
		bar_window_class.lpszClassName,
		bar_window_class.lpszClassName,
		WS_POPUP | WS_VISIBLE,
		bar_x, bar_y, bar_width, bar_thickness,
		NULL,
		NULL,
		state->hInstance,
		NULL
	);
	state->IgnoreWindows.buffer[state->IgnoreWindows.count] = bar_window;
	state->IgnoreWindows.count += 1;

	SetTimer(bar_window, 1, 2, NULL);
}


static BreezePluginCleanupFunction(DefaultBarCleanup) {
	if (background_brush) {
		DeleteObject(background_brush);
	}
	if (font) {
		DeleteObject(font);
	}
	KillTimer(bar_window, 1);
	UnregisterClassW(bar_window_class.lpszClassName, bar_window_class.hInstance);
}

static BreezePluginWorkspaceChangeCallback(DefaultBarWorkspaceCallback) {
	current_workspace = new_workspace;
	InvalidateRect(bar_window, NULL, TRUE);
}

static LRESULT CALLBACK DefaultBreezeBarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_TIMER) {
		WORD last_minute = system_time.wMinute;
		GetLocalTime(&system_time);
		if (system_time.wMinute != last_minute) {
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;
	} else if (msg == WM_PAINT) { 
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps);

		FillRect(dc, &(ps.rcPaint), background_brush);

		RECT bar_rect = ps.rcPaint;
		bar_rect.left += padding;
		bar_rect.right -= padding;

		SetBkMode(dc, TRANSPARENT);
		SelectObject(dc, font);
		SetTextColor(dc, foreground_color);
		char string_buffer[128];
		snprintf(string_buffer, sizeof(string_buffer)-1, "%hhd", current_workspace);
		DrawTextA(dc, string_buffer, -1, &bar_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

		snprintf(string_buffer, sizeof(string_buffer)-1, "%02hu:%02hu", system_time.wHour, system_time.wMinute);
		DrawTextA(dc, string_buffer, -1, &bar_rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		EndPaint(hwnd, &ps);
		return 0;
	} else {
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}
