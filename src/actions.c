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
	for (i32 i = 0; i < state->Windows.count; i += 1) {
		ShowWindowAsync(state->Windows.buffer[i].handle, SW_SHOW);
	}
	ActionSpawn(state, (uptr) explorer);
	ExitProcess((UINT) arg);
	return;
}

static HotkeyFunction(ActionSetWorkspace) {
	state->current_workspace = (i8) arg;
	for (i32 i = 0; i < state->Windows.count; i += 1) {
		i32 valid_window = 1;
		for (i32 ign = 0; ign < state->IgnoreWindows.count; ign += 1) {
			if (state->Windows.buffer[i].handle == state->IgnoreWindows.buffer[ign]) {
				valid_window = 0; 
				break;
			}
		}
		if (valid_window) {
			int command = state->Windows.buffer[i].workspace == state->current_workspace ? SW_SHOW : SW_HIDE;
			ShowWindowAsync(state->Windows.buffer[i].handle, command);
		}
	}
	state->Plugins.workspace_change_callback(state->current_workspace);
	return;
}


static HotkeyFunction(ActionSendToWorkspace) {
	HANDLE current_window = GetForegroundWindow();
	i8 workspace = (i8) arg;
	if (workspace != state->current_workspace) {
		for (i32 i = 0; i < state->Windows.count; i += 1) {
			if (current_window == state->Windows.buffer[i].handle) {
				state->Windows.buffer[i].workspace = workspace;
			}
		}
		ShowWindowAsync(current_window, SW_HIDE);
	}
}
