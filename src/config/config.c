#include "c.h"
#include "lib.h"

w32(i32) RegisterHotKey(usize handle, i32 id, u32 mods, u32 code);
w32(i32) UnregisterHotKey(usize handle, i32 id);

u16 explorer[] = L"C:\\Windows\\explorer.exe file:";

static Hotkey defaultAction[] = {
	{
		.fun = spawn,
		.arg = explorer
	},
	{
		.fun = reloadConfig,
		.arg = NULL
	},
	{
		.fun = quit,
		.arg = NULL
	}
};

static struct {
	u32 key;
	u32 modifiers;
} defaultKeys[] = {
	{
		.key = 'E',
		.modifiers = 8
	},
	{
		.key = 'R',
		.modifiers = 8
	},
	{
		.key = 'Q',
		.modifiers = 1 | 2 | 4
	}
};

bool loadConfig(void) {
	for (usize i = 0; i < hotkeys_count; ++i)
		UnregisterHotKey(0, i);

	string content;
	if (io_readFile(&temp, string("breeze.conf"), &content)) {
		return 1;
	}
	Lexer lex = Lexer_create(content);
	HotkeyList hkl = parse(&lex);
	if (hkl == NULL) {
		return 1;
	} else {
		io_write(stdout, string("Loading user's configuration.\n"));
		HotkeyList n = hkl;
		usize i = 0;
		do {
			i += 1;
			n = n->link;
		} while (n != hkl);
		Hotkey *arr = unwrap(Arena_alloc(&stable, i * sizeof(Hotkey), sizeof(void*)));
		i = 0;
		do {
			bool err = 1;
			for (usize tries = 0; tries < 10; ++tries) {
				if (RegisterHotKey(0, i, n->mod, n->key)) {
					err = 0;
					break;
				}
			}
			if (err) {
				string line;
				if (string_fmtu64(&temp, n->line, &line)) {
					line = string("##");
				}
				io_write(stderr, string_build(&temp, string("Failed to register hotkey defined by the action created at line"), line, string(".\n")));
			}
			arr[i] = n->hk;
			n = n->link;
			++i;
		} while (n != hkl);
		hotkeys_count = i;
		hotkeys = arr;
	}
	Arena_free(&temp);
	return 0;
}

void loadDefaultConfig() {
	static_assert(len(defaultKeys) == len(defaultAction), "Valid default configuration.");
	io_write(stdout, string("Loading default configuration.\n"));

	for (usize i = 0; i < len(defaultKeys); ++i) {
		bool err = 0;
		for (usize tries = 0; tries < 10; ++tries) {
			if (RegisterHotKey(0, i, defaultKeys[i].modifiers, defaultKeys[i].key)) {
				break;
			}
		}
		if (err) {
			string number;
			if (string_fmtu64(&stable, i, &number)) {
				io_write(stderr, string("Failed to set a hotkey.\n"));
				continue;
			}
			io_write(stderr, string_build(&temp, string("Failed to set default keybind no. "), number, string(".\n")));
		}
	}
	hotkeys = defaultAction;
	hotkeys_count = len(defaultKeys);
	Arena_free(&temp);
}
