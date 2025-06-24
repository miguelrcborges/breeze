#include "c.h"

#include "default_bar.c"
#include "stubs.c"

typedef struct VisitMonitorData {
	BreezeState *state;
	HMONITOR main_monitor_handle;
	i8 monitor_index;
} VisitMonitorData;

static int SortHotkeys(const void *_hk1, const void *_hk2);
static BOOL VisitMonitors(HMONITOR monitor_handle, HDC dc, LPRECT monitor_rect, LPARAM visit_monitor_data);

static u16 file_manager[] = L"explorer.exe file:";
// static u16 terminal[] = L"conhost.exe -- cmd /k cd %USERPROFILE%";
static u16 terminal[] = L"alacritty.exe";
static u16 applauncher[] = L"smenu.exe";
static u16 quickshot[] = L"quickshot.exe";

#define TAG(n) {.hotkey_value = HK_MOD_WIN | ('0' + (n)), .function = ActionSetWorkspace, .argument = n}, \
	{.hotkey_value = HK_MOD_SHIFT | HK_MOD_WIN | ('0' + (n)), .function = ActionSendToWorkspace, .argument = n}

static Hotkey default_hotkeys[] = {
	{
		.hotkey_value = HK_MOD_WIN | 'E',
		.function = ActionSpawn,
		.argument = (uptr) file_manager
	},
	{
		.hotkey_value = HK_MOD_WIN | 'R',
		.function = LoadConfig,
		.argument = 0 
	},
	{
		.hotkey_value = HK_MOD_WIN | 'C',
		.function = ActionKill,
		.argument = 0 
	},
	{
		.hotkey_value = HK_MOD_CTRL | HK_MOD_SHIFT | HK_MOD_WIN | 'Q',
		.function = ActionQuit,
		.argument = 0,
	},
	{
		.hotkey_value = HK_MOD_WIN | HK_MOD_SHIFT | 'S',
		.function = ActionSpawn,
		.argument = (uptr) quickshot,
	},
	{
		.hotkey_value = HK_MOD_WIN | VK_RETURN,
		.function = ActionSpawn,
		.argument = (uptr) terminal
	},
	{
		.hotkey_value = HK_MOD_WIN | ' ',
		.function = ActionSpawn,
		.argument = (uptr) applauncher
	},
	TAG(0),
	TAG(1),
	TAG(2),
	TAG(3),
	TAG(4),
	TAG(5),
	TAG(6),
	TAG(7),
	TAG(8),
	TAG(9),
};


static void LoadConfig(BreezeState *state, uptr unused) {
	for (i32 i = 0; i < state->Plugins.count; i += 1) {
		state->Plugins.cleanup_function[i]();
	}
	state->Plugins.count = 0;
	state->PluginFlags = 0;
	VisitMonitorData visit_monitor_data = {0};
	visit_monitor_data.state = state;
	visit_monitor_data.main_monitor_handle = MonitorFromPoint((POINT){0, 0}, MONITOR_DEFAULTTOPRIMARY);
	EnumDisplayMonitors(0, NULL, VisitMonitors, (LPARAM) &visit_monitor_data);
	state->Monitors.count = visit_monitor_data.monitor_index;

	memcpy(&(state->Hotkeys.buffer), default_hotkeys, sizeof(default_hotkeys));
	state->Hotkeys.count = len(default_hotkeys);

	// TODO: Load all plugins
	if (!(state->PluginFlags & PLUGIN_BAR)) {
		state->Plugins.cleanup_function[state->Plugins.count] = BreezePluginCleanupStub;
		SetupDefaultBar(state);
		state->Plugins.count += 1;
	}
	qsort(&(state->Hotkeys.buffer), state->Hotkeys.count, sizeof(state->Hotkeys.buffer[0]), SortHotkeys);
}


static BOOL VisitMonitors(HMONITOR monitor_handle, HDC unused_dc, LPRECT monitor_rect, LPARAM visit_monitor_data) {
	VisitMonitorData *data = (VisitMonitorData *)visit_monitor_data;
	MONITORINFOEXW mi;
	mi.cbSize = sizeof(MONITORINFOEX);
	i32 dpi = 96;
	if (GetMonitorInfoW(monitor_handle, (LPMONITORINFO)&mi)) {
		HDC dc = CreateDCW(NULL, mi.szDevice, NULL, NULL);
		if (dc) {
			dpi = GetDeviceCaps(dc, LOGPIXELSX);
		}
	}
	data->state->Monitors.buffer[data->monitor_index] = (Monitor) {
		.width = monitor_rect->right - monitor_rect->left,
		.height = monitor_rect->bottom - monitor_rect->top,
		.dpi = dpi,
		.monitor_area = *monitor_rect
	};

	SystemParametersInfoW(SPI_SETWORKAREA, 0, monitor_rect, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
	data->state->Monitors.main_monitor_index = monitor_handle == data->main_monitor_handle ? data->monitor_index : data->state->Monitors.main_monitor_index;
	data->monitor_index += 1;
	return TRUE;
}


static int SortHotkeys(const void *_hk1, const void *_hk2) {
	Hotkey *hk1 = (Hotkey *)_hk1;
	Hotkey *hk2 = (Hotkey *)_hk2;
	return (int)hk1->hotkey_value - (int)hk2->hotkey_value;
}
