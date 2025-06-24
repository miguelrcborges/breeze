#include "c.h"

#include "actions.c"
#include "config/config.c"

static BreezeState breezeState;
static HRESULT (*p_DwmGetWindowAttribute)(HWND hwnd, DWORD dwm_attribute, PVOID attribute_p, DWORD size_attribute);

static BOOL CALLBACK AddCurrentTopLevelWindows(HWND hwnd, LPARAM lParam);
static void CALLBACK WindowHandlerCallback(HWINEVENTHOOK eh, DWORD ev, HWND hwnd, LONG obj_id, LONG child_id, DWORD thread_id, DWORD dwm_time);
static LRESULT CALLBACK KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam);
static void SetupFunctionPointers(void);
static void SetProcessDPI(void);
static void KillExplorerRelatedProcesses(void);
static i8x2 GetDwmBordersStub(HWND hwnd);
static i8x2 GetDwmBordersDwmQuery(HWND hwnd);
static i8x2 (*GetDwmBorders)(HWND hwnd) = GetDwmBordersStub;


int WinMain(
	HINSTANCE instance, 
	HINSTANCE prev_instance,
	LPSTR cmd_line,
	int n_show_cmd
) {
	breezeState.current_workspace = 1;
	breezeState.hInstance = instance;

	SetProcessDPI();
	KillExplorerRelatedProcesses();
	SetupFunctionPointers();

	HWINEVENTHOOK windows_hook = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_SHOW, NULL, WindowHandlerCallback, 0, 0, WINEVENT_OUTOFCONTEXT);
	EnumWindows(AddCurrentTopLevelWindows, 0);

	HHOOK keyboard_hook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardHookCallback, NULL, 0);

	LoadConfig(&breezeState, 0);

	MSG msg;
	while (GetMessageW(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return 0;
}


static BOOL CALLBACK AddCurrentTopLevelWindows(HWND hwnd, LPARAM lParam) {
	if (IsWindowVisible(hwnd)) {
		RECT rect;
		GetClientRect(hwnd, &rect);
		if ((rect.right - rect.left) > 0 && (rect.bottom - rect.top) > 0) {
			i32 i = breezeState.Windows.count;
			breezeState.Windows.buffer[i].handle = hwnd;
			breezeState.Windows.buffer[i].workspace = 1; 
			breezeState.Windows.buffer[i].dwm_border = GetDwmBorders(hwnd);
			breezeState.Windows.buffer[i].monitor_index = breezeState.Monitors.main_monitor_index;
			breezeState.Windows.count += 1;
		}
	}
	return TRUE;
}


static void WindowHandlerCallback(HWINEVENTHOOK eh, DWORD ev, HWND hwnd, LONG obj_id, LONG child_id, DWORD thread_id, DWORD dwm_time) {
	if (obj_id == OBJID_WINDOW && GetParent(hwnd) == NULL) {
		if (ev == EVENT_OBJECT_SHOW) {
			int window_existed = 0;
			for (i32 i = 0; i < breezeState.Windows.count; i += 1) {
				if (hwnd == breezeState.Windows.buffer[i].handle) {
					window_existed = 1;
					break;
				}
			}
			if (!window_existed) {
				RECT rect;
				GetClientRect(hwnd, &rect);
				if ((rect.right - rect.left) > 0 && (rect.bottom - rect.top) > 0) {
					i32 i = breezeState.Windows.count;
					breezeState.Windows.buffer[i].handle = hwnd;
					breezeState.Windows.buffer[i].workspace = breezeState.current_workspace;
					breezeState.Windows.buffer[i].dwm_border = GetDwmBorders(hwnd);
					breezeState.Windows.count += 1;
				}
			}
		} else {
			for (i32 i = 0; i < breezeState.Windows.count; i += 1) {
				if (hwnd == breezeState.Windows.buffer[i].handle) {
					for (i32 ii = i; ii == breezeState.Windows.count - 1; ii += 1) {
						breezeState.Windows.buffer[ii] = breezeState.Windows.buffer[ii+1];
					}
					breezeState.Windows.count -= 1;
				}
			}
		}
	}
}


static LRESULT CALLBACK KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
	KBDLLHOOKSTRUCT *input_data = (KBDLLHOOKSTRUCT *)lParam;
	DWORD key = input_data->vkCode;
	DWORD is_keyup = (1 << 7) & input_data->flags;
	u16 input_mask = 0;
	input_mask = key == VK_LSHIFT || key == VK_RSHIFT || key == VK_SHIFT ? HK_MOD_SHIFT : input_mask;
	input_mask = key == VK_LCONTROL || key == VK_RCONTROL || key == VK_CONTROL ? HK_MOD_CTRL : input_mask;
	input_mask = key == VK_LMENU ? HK_MOD_ALT : input_mask;
	input_mask = key == VK_LWIN || key == VK_RWIN ? HK_MOD_WIN : input_mask;

	LRESULT handled = 0;
	if (is_keyup) {
		breezeState.Hotkeys.modifiers_state &= ~input_mask;
	} else if (input_mask) {
		breezeState.Hotkeys.modifiers_state |= input_mask;
	} else {
		u16 hotkey_value = breezeState.Hotkeys.modifiers_state | ((u16) key);
		i32 low = 0;
		i32 high = breezeState.Hotkeys.count;
		while (low < high) {
			i32 mid = (low + high) / 2;
			if (hotkey_value > breezeState.Hotkeys.buffer[mid].hotkey_value) {
				low = mid + 1;
			} else if (hotkey_value < breezeState.Hotkeys.buffer[mid].hotkey_value) { 
				high = mid;
			} else {
				breezeState.Hotkeys.buffer[mid].function(&breezeState, breezeState.Hotkeys.buffer[mid].argument);
				handled = 1;
				break;
			}
		}
	}

	if (!handled) {
		handled = CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	return handled;
}



static void SetupFunctionPointers(void) {
	HMODULE dwmapi_dll = LoadLibraryA("dwmapi.dll");
	if (dwmapi_dll) {
		typedef HRESULT (*DwmGetWindowAttribute_t)(HWND, DWORD, PVOID, DWORD);
		DwmGetWindowAttribute_t f = (DwmGetWindowAttribute_t) GetProcAddress(dwmapi_dll, "DwmGetWindowAttribute");
		if (f) {
			p_DwmGetWindowAttribute = f;
			GetDwmBorders = GetDwmBordersDwmQuery;
		}
	}
}


static void SetProcessDPI(void) {
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
	set_process_dpi_aware fun2 = (set_process_dpi_aware) GetProcAddress(user32_dll, "SetProcessDPIAware");
	if (fun2) {
		fun2();
	}

freedll:
	FreeLibrary(user32_dll);
}


static void KillExplorerRelatedProcesses(void) {
	static const char *processes_to_kill[] = {
		"explorer.exe",
		"SearchApp.exe",
		"TextInputHost.exe",
		"ShellExperienceHost.exe",
	};

	HANDLE snapshot = CreateToolhelp32Snapshot(2, 0);

	if (snapshot) {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(snapshot, &pe32)) {
			do {
				for (usize p = 0; p < len(processes_to_kill); ++p) {
					if (strcmp(pe32.szExeFile, processes_to_kill[p]) == 0) {
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


static i8x2 GetDwmBordersStub(HWND hwnd) {
	i8x2 r = {0};
	return r;
}


static i8x2 GetDwmBordersDwmQuery(HWND hwnd) {
	i8x2 r = {0};
	RECT frame_rect;
	HRESULT hr = p_DwmGetWindowAttribute(hwnd, 9, &frame_rect, sizeof(frame_rect));
	if (FAILED(hr)) {
		return r;
	}

	RECT window_rect;
	GetWindowRect(hwnd, &window_rect);

	r.x = (i8) (window_rect.left - frame_rect.left);
	r.y = (i8) (window_rect.top - frame_rect.top);
	return r;
}
