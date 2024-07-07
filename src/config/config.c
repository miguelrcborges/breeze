#include "c.h"
#include <shlobj.h>
#include <wchar.h>

#include "lex.c"
#include "map.c"
#include "parser.c"

static u16 explorer[] = L"explorer.exe file:";
static u16 terminal[] = L"conhost.exe -- cmd /k cd %USERPROFILE%";
static u16 userapps[512];
static u16 systemapps[512];

#define VDESKTOP(n, k) {.fun = switchToDesktop, .arg = (void *) n, .key = k, .mod = MOD_WIN}, {.fun = sendToDesktop, .arg = (void *) n, .key = k, .mod = MOD_WIN | MOD_SHIFT}

void loadUserApplicationDirs(void) {
	u16 tmp_buff[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_STARTMENU, NULL, SHGFP_TYPE_CURRENT, tmp_buff);
	swprintf(userapps, len(userapps) - 1, L"explorer.exe \"%s\\Programs\"", tmp_buff);
	SHGetFolderPathW(NULL, CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_CURRENT, tmp_buff);
	swprintf(systemapps, len(systemapps) - 1, L"explorer.exe \"%s\\Programs\"", tmp_buff);
}

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
	VDESKTOP(0, '1'),
	VDESKTOP(1, '2'),
	VDESKTOP(2, '3'),
	VDESKTOP(3, '4'),
	VDESKTOP(4, '5'),
	VDESKTOP(5, '6'),
	VDESKTOP(6, '7'),
	VDESKTOP(7, '8'),
	VDESKTOP(8, '9'),
	VDESKTOP(9, '0'),
};

bool loadConfig(void) {
	for (usize i = 0; i < hotkeys_count; ++i)
		UnregisterHotKey(0, i);

	char file_buffer[MAX_CONFIG_FILE_SIZE];
	hotkeys_count = 0;

	FILE *f = fopen("breeze.conf", "r");
	if (f == NULL) {
		puts("Config file not found.");
		return 1;
	}
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	if (len >= MAX_CONFIG_FILE_SIZE) {
		fprintf(stderr, "File too long.");
		return 1;
	}
	fseek(f, 0, SEEK_SET);
	usize read = fread(file_buffer, sizeof(char), len, f);
	file_buffer[read] = '\0';
	fclose(f);

	Lexer lex = Lexer_create(file_buffer);
	bool err = parse(&lex);
	if (err) {
		return 1;
	} else {
		puts("Loading user's configuration.");
		for (usize i = 0; i < hotkeys_count; ++i) {
			bool err = 1;
			for (usize tries = 0; tries < 10; ++tries) {
				if (RegisterHotKey(0, i, hotkeys_buf[i].mod, hotkeys_buf[i].key)) {
					err = 0;
					break;
				}
			}
			if (err) {
				fprintf(stderr, "Failed to register hotkey defined by the action created at line %llu.\n", hotkeys_buf[i].line);
			}
		}
		hotkeys = hotkeys_buf;
	}
	return 0;
}

void loadDefaultConfig() {
	puts("Loading default configuration.");

	for (usize i = 0; i < len(defaultHotkeys); ++i) {
		bool err = 0;
		for (usize tries = 0; tries < 10; ++tries) {
			if (RegisterHotKey(0, i, defaultHotkeys[i].mod, defaultHotkeys[i].key)) {
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
}
