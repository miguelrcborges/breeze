#include "c.h"


static HotkeyFunction(ActionSpawn) {
	u16 *command = (u16 *)arg;

	STARTUPINFOW si = {
		.cb = sizeof(STARTUPINFOW)
	};
	PROCESS_INFORMATION pi;

	CreateProcessW(0, command, 0, 0, 0, CREATE_NEW_PROCESS_GROUP | CREATE_NO_WINDOW, 0, 0, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

static HotkeyFunction(ActionKill) {
	PostMessageW(GetForegroundWindow(), WM_CLOSE, 0, 0);
}

static HotkeyFunction(ActionQuit) {
	u16 explorer[] = L"explorer.exe";
	for (i32 i = 0; i < state->Windows.buffer_alloc_pos; i += 1) {
		HWND w = state->Windows.buffer[i].handle;
		if (w) {
			ShowWindowAsync(w, SW_SHOW);
		}
	}
	ActionSpawn(state, (uptr) explorer);
	ExitProcess((UINT) arg);
	return;
}

static HotkeyFunction(ActionSetWorkspace) {
	i8 tw = (i8) arg;
	i8 cw = state->current_workspace;
	if (tw != cw) {
		state->Windows.last_focused_window[cw] = GetForegroundWindow();
		for (i32 m = 0; m < state->Monitors.count; m += 1) {
			u16 start_window = state->Windows.current_workspace_window[m][cw];
			u16 w = start_window;
			if (start_window) {
				do {
					ShowWindowAsync(state->Windows.buffer[w].handle, SW_HIDE);
					w = state->Windows.buffer[w].next_workspace_window;
				} while (w != start_window);
			}
			start_window = state->Windows.current_workspace_window[m][tw];
			w = start_window;
			if (start_window) {
				do {
					ShowWindowAsync(state->Windows.buffer[w].handle, SW_SHOW);
					w = state->Windows.buffer[w].next_workspace_window;
				} while (w != start_window);
			}
		}
	}
	state->current_workspace = tw;
	state->Plugins.workspace_change_callback(tw);
	SetActiveWindow(state->Windows.last_focused_window[tw]);
	return;
}


static HotkeyFunction(ActionSendToWorkspace) {
	HANDLE current_window = GetForegroundWindow();
	i8 cw = state->current_workspace;
	i8 workspace = (i8) arg;
	if (current_window && workspace != cw) {
		i8 m;
		for (m = 0; m < state->Monitors.count; m += 1) {
			u16 sw = state->Windows.current_workspace_window[m][cw];
			u16 w = sw;
			int should_break = 0;
			do {
				if (current_window == state->Windows.buffer[w].handle) {
					state->Windows.buffer[w].workspace = workspace;
					u16 p = state->Windows.buffer[w].prev_workspace_window;
					u16 n = state->Windows.buffer[w].next_workspace_window;
					state->Windows.buffer[p].next_workspace_window = n;
					state->Windows.buffer[n].prev_workspace_window = p;
					u16 t = state->Windows.current_workspace_window[m][workspace];
					if (t) {
						u16 tn = state->Windows.buffer[t].next_workspace_window;
						state->Windows.buffer[w].next_workspace_window = tn;
						state->Windows.buffer[w].prev_workspace_window = t;
						state->Windows.buffer[t].next_workspace_window = w;
						state->Windows.buffer[tn].prev_workspace_window = w;
					} else {
						state->Windows.current_workspace_window[m][workspace] = w;
						state->Windows.buffer[w].next_workspace_window = w;
						state->Windows.buffer[w].prev_workspace_window = w;
					}
					should_break = 1;
					break;
				}
				w = state->Windows.buffer[w].next_workspace_window;
			} while (w != sw);
			if (should_break) break;
		}
		ShowWindowAsync(current_window, SW_HIDE);
	}
}
