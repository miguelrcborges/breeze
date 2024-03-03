#include "c.h"

w32(bool) UnregisterHotKey(usize handle, i32 id);
w32(void) ExitProcess(u32 code);
w32(usize) CreateFileA(u8 *fname, u32 flags, u32 shared, void *sec, u64 modes, u64 attributes, int);
w32(u32) GetFileSize(usize fd, u32 *high);
w32(bool) ReadFile(usize fd, const u8 *buffer, u32 len, u32 *written, void *overlapped);
w32(bool) CloseHandle(usize handle);

typedef struct hotkeyList HotkeysList;
struct hotkeyList {
	Hotkeys action;
	HotkeysList *next;
	u32 key;
	u32 modifider;
};

static usize cursor;
static usize line;
static string config;
static bool has_error;

static bool peek() {
	if (cursor > config.len) {
		return '0';
	}
	return config.str[cursor];
}

static bool isAlpha(u8 ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static bool isWhiteSpace(u8 ch) {
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\f' || ch == '\v';
}

static void readFile() {
	usize fd = CreateFileA((u8 *)"./breeze.conf", 1179785, 0, 0, 3, 128, 0);
	if (unlikely(fd == (usize) -1)) {
		io_write(stderr, string("Failed to open configuration file.\n"));
		return;
	}
	
	u32 hw;
	u32 lw = GetFileSize(fd, &hw);
	usize len = ((usize) hw << 32) | (usize) lw;
	if (unlikely(len >= ((usize) 1 << 32))) {
		io_write(stderr, string("Files longer than 4 GB not read because dev skill issue.\n"));
		ExitProcess(-1);
	}

	if (len >= config.len) {
		if (unlikely(mem_release((void *)config.str, config.len))) {
			io_write(stderr, string("Failed to free memory.\n"));
			ExitProcess(-1);
		}

		config.len = len;
		SafePointer sp = mem_rescommit(config.len);
		if (sp._ptr == NULL) {
			io_write(stderr, string("Failed to allocate memory.\n"));
			ExitProcess(-1);
		}
		config.str = sp._ptr;
	} 

	u32 read;
	if (!ReadFile(fd, config.str, config.len, &read, NULL)) {
		io_write(stderr, string("Failed to read file.\n"));
		ExitProcess(-1);
	};

	CloseHandle(fd);
}

static void skipWhitespace() {
	u8 ch;
	while (isWhiteSpace(ch = peek())) {
		if (ch == '\n')
			++line;
		++cursor;
	}
}

static void ignoreScope() {
	u8 ch;

	skipWhitespace();
	ch = peek();
	if (ch == '{') {
		do {
			++cursor;
			ch = peek();
		} while (ch != '}' && ch != '\0');

		if (ch == '\0') {
			io_write(stderr, string("Unfinished scope at end of file.\n"));
			has_error = 1;
		} else {
			++cursor;
		}
	}
}

bool loadConfig(void) {
	has_error = 0;
	line = 1;

	for (usize i = 0; i < hotkeys_count; ++i) {
		UnregisterHotKey(0, (i32) i);
	}

	readFile();
	cursor = 0;
	while (cursor <= config.len) {
		skipWhitespace();

		u8 ch = peek();
		if (unlikely(ch == '\0')) {
			break;
		} else if (!isAlpha(ch)) {
			// Need to implement string formatting in my lib
			io_write(stderr, string("Invalid character in an action position at line ##\n"));
			has_error = 1;
		} else {
			usize start = cursor;
			do {
				++cursor;
			} while (isAlpha(peek()));
			string action = (string) {
				.str = config.str + start,
				.len = cursor - start
			};

			switch (action.str[0]) {
				case 's': {
					if (string_compare(action, string("spawn"))) {
						ignoreScope();
					} else {
						goto unknown;
					}
					break;
				}
				default:
				unknown: {
					// I really need to implement formatting
					io_write(stderr, string("Unkown action named \""));
					io_write(stderr, action);
					io_write(stderr, string("\" present in line ##.\n"));
					has_error = 1;
					ignoreScope();
				}
			}
		}

		++cursor;
	}

	return has_error;
}
