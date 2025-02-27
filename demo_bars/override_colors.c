#include "../src/c.h"

#define dll_export __declspec(dllexport)

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	return true;
}

dll_export void BreezePluginSetup(BreezeState *state) {
	state->bar.foreground = 0x0022aa22;
	state->bar.background = 0x00226622;
}
