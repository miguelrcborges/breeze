#include "c.h"

w32(i32) RegisterHotKey(usize handle, i32 id, u32 mods, u32 code);
w32(i32) UnregisterHotKey(usize handle, i32 id);

u16 explorer[] = L"explorer.exe file:";

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

	string content = io_readFile(&temp, str("breeze.conf"));
	Lexer lex = Lexer_create(content);
	HotkeyList hkl = parse(&lex);
	if (hkl == NULL) {
		return 1;
	} else {
		io_write(stdout, str("Loading user's configuration.\n"));
		HotkeyList n = hkl;
		usize i = 0;
		do {
			i += 1;
			n = n->link;
		} while (n != hkl);
		Hotkey *arr = Arena_alloc(&stable, i * sizeof(Hotkey), sizeof(void*));
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
				io_write(stderr, string_build(&temp, str("Failed to register hotkey defined by the action created at line "),
					string_fmtu64(&temp, n->line), str(".\n")));
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
	static_assert(len(defaultKeys) == len(defaultAction));
	io_write(stdout, str("Loading default configuration.\n"));

	for (usize i = 0; i < len(defaultKeys); ++i) {
		bool err = 0;
		for (usize tries = 0; tries < 10; ++tries) {
			if (RegisterHotKey(0, i, defaultKeys[i].modifiers, defaultKeys[i].key)) {
				break;
			}
		}
		if (err) {
			io_write(stderr, string_build(&temp, str("Failed to set default keybind no. "), string_fmtu64(&stable, i), str(".\n")));
		}
	}
	hotkeys = defaultAction;
	hotkeys_count = len(defaultKeys);
	Arena_free(&temp);
}
