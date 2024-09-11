#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	MAX_KEYBOARD_LAYOUTS = 16,
};

static HKL layouts[MAX_KEYBOARD_LAYOUTS];

int main(void) {
	int layouts_count = GetKeyboardLayoutList(MAX_KEYBOARD_LAYOUTS, layouts);

	puts("Available keyboard layouts:");
	for (int i = 0; i < layouts_count; i += 1) {
		WCHAR localeName[LOCALE_NAME_MAX_LENGTH];
		LANGID langId = LOWORD(layouts[i]);
		LCID localeId = MAKELCID(langId, SORT_DEFAULT);
		if (LCIDToLocaleName(localeId, localeName, LOCALE_NAME_MAX_LENGTH, 0)) {
			wprintf(L"\t%d) %s\n", i+1, localeName);
		}
	}

	int input;
	for (;;) {
		printf("Insert desired keyboard layout: ");
		char buf[1024];
		if (!fgets(buf, 1024, stdin)) {
			return 1;
		}
		input = atoi(buf);
		if (input > 0 && input <= layouts_count) break;
		puts("Reply with a valid input.");
	}

	int layout_to_set = input - 1;
	SendMessageW(HWND_BROADCAST, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM) layouts[layout_to_set]);

	return 0;
}
