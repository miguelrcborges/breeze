#include "c.h"
#include <shlobj.h>
#include <wchar.h>

#include "lex.c"
#include "map.c"
#include "parser.c"


static void loadDefaultConfig();
static void setBarDefaults();

static u16 explorer[] = L"explorer.exe file:";
static u16 terminal[] = L"conhost.exe -- cmd /k cd %USERPROFILE%";
static u16 userapps[512];
static u16 systemapps[512];
static bool hasConfigError;

#define VDESKTOP(n) {.fun = switchToDesktop, .arg = (void *) n, .key = ('0'+n), .mod = MOD_WIN}, {.fun = sendToDesktop, .arg = (void *) n, .key = ('0'+n), .mod = MOD_WIN | MOD_SHIFT}
static Hotkey defaultHotkeys[] = {
	{
		.fun = spawn,
		.arg = explorer,
		.key = 'E',
		.mod = MOD_WIN
	},
	{
		.fun = reloadConfig,
		.arg = NULL,
		.key = 'R',
		.mod = MOD_WIN
	},
	{
		.fun = quit,
		.arg = NULL,
		.key = 'Q',
		.mod = MOD_CONTROL | MOD_SHIFT | MOD_ALT
	},
	{
		.fun = kill,
		.arg = NULL,
		.key = 'C',
		.mod = MOD_WIN 
	},
	{
		.fun = spawn,
		.arg = terminal,
		.key = VK_RETURN,
		.mod = MOD_CONTROL
	},
	{
		.fun = spawn,
		.arg = userapps,
		.key = 'A',
		.mod = MOD_WIN,
	},
	{
		.fun = spawn,
		.arg = systemapps,
		.key = 'A',
		.mod = MOD_WIN | MOD_SHIFT,
	},
	VDESKTOP(1),
	VDESKTOP(2),
	VDESKTOP(3),
	VDESKTOP(4),
	VDESKTOP(5),
	VDESKTOP(6),
	VDESKTOP(7),
	VDESKTOP(8),
	VDESKTOP(9),
	VDESKTOP(0),
};
#undef VDESKTOP

void loadUserApplicationDirs(void) {
	u16 tmp_buff[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, tmp_buff);
	swprintf(userapps, len(userapps) - 1, L"explorer.exe \"%s\"", tmp_buff);
	SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, tmp_buff);
	swprintf(systemapps, len(systemapps) - 1, L"explorer.exe \"%s\"", tmp_buff);
}


void loadConfig(void) {
	hasConfigError = 0;
	for (usize i = 0; i < hotkeys_count; ++i)
		UnregisterHotKey(bar_window, i);

	char file_buffer[MAX_CONFIG_FILE_SIZE];
	hotkeys_count = 0;

	FILE *f = fopen("breeze.conf", "r");
	if (f == NULL) {
		loadDefaultConfig();
		return;
	}

	FILE *logs_file = fopen("breeze.log", "w");
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	if (len >= MAX_CONFIG_FILE_SIZE) {
		registerError(logs_file, "File too long.");
		loadDefaultConfig();
		if (likely(logs_file)) fclose(logs_file);
		return;
	}

	fseek(f, 0, SEEK_SET);
	usize read = fread(file_buffer, sizeof(char), len, f);
	file_buffer[read] = '\0';
	fclose(f);

	setBarDefaults();
	widestringAllocator.position = 0;
	Lexer lex = Lexer_create(file_buffer, logs_file);
	parse(&lex, logs_file);
	if (likely(logs_file)) fclose(logs_file);
	if (hasConfigError) {
		loadDefaultConfig();
	} else {
		puts("Loading user's configuration.");
		for (usize i = 0; i < hotkeys_count; ++i) {
			bool err = 1;
			for (usize tries = 0; tries < 10; ++tries) {
				if (RegisterHotKey(bar_window, i, hotkeys_buf[i].mod, hotkeys_buf[i].key)) {
					err = 0;
					break;
				}
			}
			if (err) {
				char buffer[1024];
				snprintf(buffer, len(buffer)-1, "Failed to register hotkey defined by the action created at line %u.\n", hotkeys_buf[i].line);
				MessageBoxA(NULL, buffer, "Breeze Adding Hotkey Error", MB_OK | MB_ICONWARNING);
			}
		}
		hotkeys = hotkeys_buf;
	}
	return;
}

static void loadDefaultConfig() {
	for (usize i = 0; i < len(defaultHotkeys); ++i) {
		bool err = 0;
		for (usize tries = 0; tries < 10; ++tries) {
			if (RegisterHotKey(bar_window, i, defaultHotkeys[i].mod, defaultHotkeys[i].key)) {
				break;
			}
		}
		if (err) {
			fprintf(stderr, "Failed to set default keybind no. %llu.\n", i);
			exit(1);
		}
	}
	hotkeys = defaultHotkeys;
	hotkeys_count = len(defaultHotkeys);
	setBarDefaults();
}


static void setBarDefaults() {
	if (bar_font != default_bar_font) DeleteObject(bar_font);
	foreground = BAR_DEFAULT_FOREGROUND;
	background = BAR_DEFAULT_BACKGROUND;
	bar_font_height = BAR_DEFAULT_FONT_HEIGHT;
	bar_font = default_bar_font;
	bar_position = BAR_RIGHT;
	bar_width = BAR_DEFAULT_WIDTH;
	bar_pad = BAR_DEFAULT_PAD; 
}



void registerError(FILE *logs_file, const char *error_fmt, ...) {
	va_list args;
	va_start(args, error_fmt);

	char error_msg[512];
	int length = vsnprintf(error_msg, len(error_msg)-1, error_fmt, args);
	va_end(args);

	if (likely(logs_file)) {
		fwrite(error_msg, 1, length, logs_file);
	}

	if (unlikely(!hasConfigError)) {
		hasConfigError = true;
		char message_buffer[1024];
		snprintf(message_buffer, len(message_buffer)-1, "First configuration error:\n%s\nFor full error message, check breeze.log.", error_msg);
		MessageBoxA(NULL, message_buffer, "Breeze User's Configuration Error", MB_OK | MB_ICONWARNING);
	}
}
