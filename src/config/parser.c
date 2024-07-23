#include "c.h"

#include "stdio.h"
#include <shlobj.h>

static Lexer *l;
static bool err;
static bool has_desktops;

static char *tokenStrings[TOKEN_COUNT] = {
	[TOKEN_INVALID] = "invalid",
	[TOKEN_EOF] = "end of file",
	[TOKEN_UNTERMINATED_STRING] = "unterminated string",
	[TOKEN_LBRACE] = "left brace",
	[TOKEN_RBRACE] = "right brace",
	[TOKEN_PLUS] = "+ operator",
	[TOKEN_EQUAL] = "= operator",
	[TOKEN_ACTION] = "action",
	[TOKEN_ACTION_ATTRIBUTE] = "action attribute",
	[TOKEN_KEY] = "key token",
	[TOKEN_STRING] = "string",
	[TOKEN_MODIFIER] = "modifier token",
	[TOKEN_DESKTOPS] = "desktops token",
	[TOKEN_DESKTOPS_ATTRIBUTE] = "desktops attribute token",
};

static bool actionRequiresArg[ACTION_COUNT] = {
	[ACTION_SPAWN] = 1,
};

static void (*actionMap[ACTION_COUNT])(void *arg) = {
	[ACTION_SPAWN] = spawn,
	[ACTION_RELOAD] = reloadConfig,
	[ACTION_QUIT] = quit,
	[ACTION_KILL] = kill,
	[ACTION_FOCUS_NEXT] = focusNext,
	[ACTION_FOCUS_PREV] = focusPrev,
	[ACTION_REVEAL_ALL] = revealAllWindows,
};

static char *actionStrings[ACTION_COUNT] = {
	[ACTION_SPAWN] = "spawn",
	[ACTION_RELOAD] = "reload",
	[ACTION_QUIT] = "quit",
	[ACTION_KILL] = "kill",
	[ACTION_FOCUS_NEXT] = "focus next",
	[ACTION_FOCUS_PREV] = "focus previous",
	[ACTION_REVEAL_ALL] = "reveal all windows",
};

typedef struct {
	u32 send;
	u32 _switch;
} DesktopsMods;


static void parseAction(uptr action);
static void parseActionAttribute(uptr action, uptr attr);
static void parseDesktops(void);
static DesktopsMods parseDesktopsAttribute(DesktopsMods mods, uptr attr);

bool parse(Lexer *lex) {
	Token t;
	err = 0;
	has_desktops = 0;
	l = lex;
	for (;;) {
		t = Lexer_nextToken(lex);
		switch (t.type) {
			case TOKEN_ACTION: {
				parseAction(t.value);
				break;
			}
			case TOKEN_DESKTOPS: {
				if (has_desktops) {
					fprintf(stderr, "Error: Two desktops scopes defined.\n");
					err = 1;
					goto end;
				}
				has_desktops = 1;
				parseDesktops();
				break;
			}
			case TOKEN_UNTERMINATED_STRING: {
				fprintf(stderr, "Unterminated string in the configuration file at an action position.\n");
				err = 1;
				goto end;
			}
			case TOKEN_EOF: {
				goto end;
			}
			default: {
				fprintf(stderr, "Expected action, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				err = 1;
			}
		}
	}
end:
	return err;
}

static void parseAction(uptr action) {
	hotkeys_buf[hotkeys_count] = (Hotkey){.fun = actionMap[action], .line = l->line}; // Zeroes previous state
	Token t = Lexer_nextToken(l);
	if (t.type != TOKEN_LBRACE) {
		fprintf(stderr, "Expected a left brace, got %s at line %llu.\n", tokenStrings[t.type], l->line);
		err = 1;
		return;
	}

	for (;;) {
		t = Lexer_nextToken(l); 
		switch (t.type) {
			case TOKEN_ACTION_ATTRIBUTE: {
				parseActionAttribute(action, t.value);
				break;
			}
			case TOKEN_EOF: {
				fprintf(stderr, "Unclosed action scope opened at line %llu.\n", hotkeys_buf[hotkeys_count].line);
				err = 1;
				goto end;
			}
			case TOKEN_RBRACE: {
				goto exit_loop;
			}
			default: {
				fprintf(stderr, "Expected an action attribute, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				err = 1;
			}
		}
	}
exit_loop:
	if (!likely(hotkeys_buf[hotkeys_count].key && hotkeys_buf[hotkeys_count].mod && (!actionRequiresArg[action] || hotkeys_buf[hotkeys_count].arg))) {
		fprintf(stderr, "Not enough attributes defined in the action created at line %llu.\n", hotkeys_buf[hotkeys_count].line);
		err = 1;
	} else {
		hotkeys_count += 1;
	}
end:
	return;
}

static void parseActionAttribute(uptr action, uptr attr) {
	Token t = Lexer_nextToken(l);
	if (t.type != TOKEN_EQUAL) {
		err = 1;
		fprintf(stderr, "Expected a = operator, got %s at line %llu.\n", tokenStrings[t.type], l->line);
		return;
	}
	t = Lexer_nextToken(l);
	switch (attr) {
		case ATTRIBUTE_KEY: {
			if (t.type != TOKEN_KEY) {
				err = 1;
				fprintf(stderr, "Expected a key token, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				return;
			}
			u32 tmpkey = t.value;
			if (Lexer_peekToken(l).type == TOKEN_PLUS) {
				err = 1;
				fprintf(stderr, "You can only assign one key per hotkey. Error at line %llu.\n", l->line);
				return;
			}
			hotkeys_buf[hotkeys_count].key = tmpkey;
			return;
		}
		case ATTRIBUTE_MODIFIER: {
			if (t.type != TOKEN_MODIFIER) {
				err = 1;
				fprintf(stderr, "Expected a modifier token, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				return;
			}
			hotkeys_buf[hotkeys_count].mod = t.value;
			while (Lexer_peekToken(l).type == TOKEN_PLUS) {
				Lexer_nextToken(l);
				t = Lexer_nextToken(l);
				if (t.type != TOKEN_MODIFIER) {
					err = 1;
					fprintf(stderr, "Expected a modifier token after +, got %s at line %llu.\n", tokenStrings[t.type], l->line);
					return;
				}
				hotkeys_buf[hotkeys_count].mod |= t.value;
			}
			return;
		}
		case ATTRIBUTE_ARG: {
			if (!actionRequiresArg[action]) {
				err = 1;
				fprintf(stderr, "The action %s, created at line %llu, musn't have an arg attribute.\n", actionStrings[action], hotkeys_buf[hotkeys_count].line);
				return;
			}
			if (t.type != TOKEN_STRING) {
				err = 1;
				fprintf(stderr, "Expected a string token, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				return;
			}
			hotkeys_buf[hotkeys_count].arg = (u16 *)t.value;
			return;
		}
	} 
}


static void parseDesktops(void) {
	usize line = l->line;
	DesktopsMods mods = {};
	Token t = Lexer_nextToken(l);
	if (t.type != TOKEN_LBRACE) {
		fprintf(stderr, "Expected a left brace, got %s at line %llu.\n", tokenStrings[t.type], l->line);
		err = 1;
		return;
	}

	for (;;) {
		t = Lexer_nextToken(l); 
		switch (t.type) {
			case TOKEN_DESKTOPS_ATTRIBUTE: {
				mods = parseDesktopsAttribute(mods, t.value);
				break;
			}
			case TOKEN_EOF: {
				fprintf(stderr, "Unclosed desktops scope opened at line %llu.\n", hotkeys_buf[hotkeys_count].line);
				err = 1;
				goto end;
			}
			case TOKEN_RBRACE: {
				goto exit_loop;
			}
			default: {
				fprintf(stderr, "Expected a desktops attribute, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				err = 1;
			}
		}
	}
exit_loop:
	if (mods.send == 0 || mods._switch == 0) {
		fprintf(stderr, "Attributes not set in desktops scope, created at line %llu.\n", line);
		err = 1;
		return;
	}
	if (mods.send == mods._switch) {
		fprintf(stderr, "Attributes in desktops scope, created at line %llu, have the same value.\n", line);
		err = 1;
		return;
	}
	for (usize i = 0; i <= 9; ++i) {
		hotkeys_buf[hotkeys_count++] = (Hotkey) {
			.arg = (void *)i,
			.line = line,
			.fun = switchToDesktop,
			.key = '0' + i,
			.mod = mods._switch
		};
		hotkeys_buf[hotkeys_count++] = (Hotkey) {
			.arg = (void *)i,
			.line = line,
			.fun = sendToDesktop,
			.key = '0' + i,
			.mod = mods.send
		};
	}
end:
	return;
}

static DesktopsMods parseDesktopsAttribute(DesktopsMods mods, uptr attr) {
	Token t = Lexer_nextToken(l);
	if (t.type != TOKEN_EQUAL) {
		err = 1;
		fprintf(stderr, "Expected a = operator, got %s at line %llu.\n", tokenStrings[t.type], l->line);
		return mods;
	}
	t = Lexer_nextToken(l);
	switch (attr) {
		case ATTRIBUTE_SEND_TO_DESKTOP: {
			if (t.type != TOKEN_MODIFIER) {
				err = 1;
				fprintf(stderr, "Expected a modifier token, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				return mods;
			}
			mods.send |= t.value;
			while (Lexer_peekToken(l).type == TOKEN_PLUS) {
				Lexer_nextToken(l);
				t = Lexer_nextToken(l);
				if (t.type != TOKEN_MODIFIER) {
					err = 1;
					fprintf(stderr, "Expected a modifier token after +, got %s at line %llu.\n", tokenStrings[t.type], l->line);
					return mods;
				}
				mods.send |= t.value;
			}
			return mods;
		}
		case ATTRIBUTE_SWITCH_TO_DESKTOP: {
			if (t.type != TOKEN_MODIFIER) {
				err = 1;
				fprintf(stderr, "Expected a modifier token, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				return mods;
			}
			mods._switch |= t.value;
			while (Lexer_peekToken(l).type == TOKEN_PLUS) {
				Lexer_nextToken(l);
				t = Lexer_nextToken(l);
				if (t.type != TOKEN_MODIFIER) {
					err = 1;
					fprintf(stderr, "Expected a modifier token after +, got %s at line %llu.\n", tokenStrings[t.type], l->line);
					return mods;
				}
				mods._switch |= t.value;
			}
			return mods;
		}
	} 

	return mods;
}
