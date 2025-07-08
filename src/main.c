#include "c.h"

#include "actions.c"
#include "config/config.c"

static BOOL CALLBACK AddCurrentTopLevelWindows(HWND hwnd, LPARAM lParam);
static void CALLBACK WindowHandlerCallback(HWINEVENTHOOK eh, DWORD ev, HWND hwnd, LONG obj_id, LONG child_id, DWORD thread_id, DWORD dwm_time);
static LRESULT CALLBACK KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam);
static void SetProcessDPI(void);
static void KillExplorerRelatedProcesses(void);

static BreezeState breezeState;


int WinMain(
	HINSTANCE instance, 
	HINSTANCE prev_instance,
	LPSTR cmd_line,
	int n_show_cmd
) {
	breezeState.current_workspace = 1;
	breezeState.hInstance = instance;
	breezeState.Windows.buffer_alloc_pos = 1;

	SetProcessDPI();
	KillExplorerRelatedProcesses();

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
			i32 i = breezeState.Windows.buffer_alloc_pos;
			breezeState.Windows.buffer[i].handle = hwnd;
			breezeState.Windows.buffer[i].workspace = 1; 
			breezeState.Windows.buffer[i].monitor_index += breezeState.Monitors.main_monitor_index;
			breezeState.Windows.buffer_alloc_pos += 1;
			for (i32 m = 0; m < breezeState.Monitors.count; m += 1) {
				RECT mr = breezeState.Monitors.buffer[m].work_area;
				bool c1 = rect.left >= mr.left && rect.top >= mr.top;
				bool c2 = rect.right <= mr.right && rect.bottom <= mr.bottom;
				breezeState.Windows.buffer[i].monitor_index = c1 && c2 ? (i8) m : breezeState.Windows.buffer[i].monitor_index;
			}

			i8 m = breezeState.Windows.buffer[i].monitor_index;
			breezeState.Windows.buffer[i].next_workspace_window = 0;
			if (breezeState.Windows.current_workspace_window[m][1] == 0) {
				breezeState.Windows.current_workspace_window[m][1] = (i8) i;
				breezeState.Windows.buffer[i].prev_workspace_window = (u16) i;
				breezeState.Windows.buffer[i].next_workspace_window = (u16) i;
			} else {
				u16 cw = breezeState.Windows.current_workspace_window[m][1];
				breezeState.Windows.buffer[i].next_workspace_window = breezeState.Windows.buffer[cw].next_workspace_window;
				breezeState.Windows.buffer[cw].next_workspace_window = (u16) i;
				breezeState.Windows.buffer[i].prev_workspace_window = (u16) cw;
			}
		}
	}
	return TRUE;
}


static void WindowHandlerCallback(HWINEVENTHOOK eh, DWORD ev, HWND hwnd, LONG obj_id, LONG child_id, DWORD thread_id, DWORD dwm_time) {
	if (obj_id == OBJID_WINDOW && GetParent(hwnd) == NULL) {
		if (ev == EVENT_OBJECT_SHOW) {
			int new_window = 1;
			int valid_window = 1;
			for (i32 i = 0; i < breezeState.Windows.buffer_alloc_pos; i += 1) {
				if (hwnd == breezeState.Windows.buffer[i].handle) {
					new_window = 0;
					breezeState.Windows.buffer[i].workspace = breezeState.current_workspace;
					break;
				}
			}
			if (new_window) {
				for (i32 i = 0; i < breezeState.IgnoreWindows.count; i += 1) {
					if (hwnd == breezeState.IgnoreWindows.buffer[i]) {
						valid_window = 0;
						break;
					}
				}
			}
			if (new_window && valid_window) {
				RECT rect;
				GetClientRect(hwnd, &rect);
				if ((rect.right - rect.left) > 0 && (rect.bottom - rect.top) > 0) {
					i32 i;
					if (breezeState.Windows.first_free_index != 0) {
						i = breezeState.Windows.first_free_index;
						breezeState.Windows.first_free_index = breezeState.Windows.buffer[i].next_workspace_window;
					} else {
						i = breezeState.Windows.buffer_alloc_pos;
						breezeState.Windows.buffer_alloc_pos += 1;
					}
					breezeState.Windows.buffer[i].handle = hwnd;
					breezeState.Windows.buffer[i].workspace = breezeState.current_workspace;
					breezeState.Windows.buffer[i].monitor_index += breezeState.Monitors.main_monitor_index;
					for (i32 m = 0; m < breezeState.Monitors.count; m += 1) {
						RECT mr = breezeState.Monitors.buffer[m].work_area;
						bool c1 = rect.left >= mr.left && rect.top >= mr.top;
						bool c2 = rect.right <= mr.right && rect.bottom <= mr.bottom;
						breezeState.Windows.buffer[i].monitor_index = c1 && c2 ? (i8) m : breezeState.Windows.buffer[i].monitor_index;
					}
					i8 m = breezeState.Windows.buffer[i].monitor_index;
					i8 w = breezeState.current_workspace;
					breezeState.Windows.buffer[i].next_workspace_window = 0;
					if (breezeState.Windows.current_workspace_window[m][w] == 0) {
						breezeState.Windows.current_workspace_window[m][w] = (i8) i;
						breezeState.Windows.buffer[i].prev_workspace_window = (u16) i;
						breezeState.Windows.buffer[i].next_workspace_window = (u16) i;
					} else {
						u16 cw = breezeState.Windows.current_workspace_window[m][w];
						breezeState.Windows.buffer[i].next_workspace_window = breezeState.Windows.buffer[cw].next_workspace_window;
						breezeState.Windows.buffer[cw].next_workspace_window = (u16) i;
						breezeState.Windows.buffer[i].prev_workspace_window = (u16) cw;
					}
					SetActiveWindow(hwnd);
				}
			}
		} else {
			for (i32 i = 0; i < breezeState.Windows.buffer_alloc_pos; i += 1) {
				if (hwnd == breezeState.Windows.buffer[i].handle) {
					i8 m = breezeState.Windows.buffer[i].monitor_index;
					i8 w = breezeState.Windows.buffer[i].workspace;
					if (breezeState.Windows.current_workspace_window[m][w] == i) {
						breezeState.Windows.current_workspace_window[m][w] = breezeState.Windows.buffer[i].next_workspace_window;
					}
					u16 p = breezeState.Windows.buffer[i].prev_workspace_window;
					u16 n = breezeState.Windows.buffer[i].next_workspace_window;
					breezeState.Windows.buffer[p].next_workspace_window = n;
					breezeState.Windows.buffer[n].prev_workspace_window = p;
					breezeState.Windows.buffer[i].handle = NULL;
					breezeState.Windows.buffer[i].next_workspace_window = breezeState.Windows.first_free_index;
					breezeState.Windows.first_free_index = (u16) i;
					break;
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

