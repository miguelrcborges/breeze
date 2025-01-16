#include "c.h"

#include "stdio.h"
#include <shlobj.h>


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
	[TOKEN_BAR] = "bar token",
	[TOKEN_BAR_ATTRIBUTE] = "bar attribute token",
	[TOKEN_COLOR] = "color",
	[TOKEN_INVALID_COLOR] = "invalid color",
	[TOKEN_NUMBER] = "number",
	[TOKEN_POSITION] = "position",
};

static bool actionRequiresArg[ACTION_COUNT] = {
	[ACTION_SPAWN] = 1,
	[ACTION_COMMAND_LINE] = 1,
};

static HotkeyFunction *actionMap[ACTION_COUNT] = {
	[ACTION_SPAWN] = spawn,
	[ACTION_COMMAND_LINE] = spawnWithoutConsole,
	[ACTION_RELOAD] = reloadConfig,
	[ACTION_QUIT] = quit,
	[ACTION_KILL] = kill,
	[ACTION_FOCUS_NEXT] = focusNext,
	[ACTION_FOCUS_PREV] = focusPrev,
	[ACTION_REVEAL_ALL] = revealAllWindows,
};

static char *actionStrings[ACTION_COUNT] = {
	[ACTION_SPAWN] = "spawn",
	[ACTION_COMMAND_LINE] = "command line",
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

typedef struct {
	Lexer *lex;
	FILE *logs_file;
	u32 flags;
} ParserState;

enum ParserStateFlags {
	PARSERSTATE_HASDESKTOP = 1 << 0,
	PARSERSTATE_HASBAR = 1 << 1,
};


static void parseAction(BreezeState *b, ParserState *s, uptr action, u32 line);
static void parseActionAttribute(BreezeState *b, ParserState *s, Hotkey *current_hotkey, uptr action, uptr attr);
static void parseDesktops(BreezeState *b, ParserState *s);
static DesktopsMods parseDesktopsAttribute(BreezeState *b, ParserState *s, DesktopsMods mods, uptr attr);
static void parseBar(BreezeState *b, ParserState *s);
static const u16 *parseBarAttribute(BreezeState *b, ParserState *s, const u16 *font_str, uptr attr);

void parse(BreezeState *breezeState, Lexer *lex, FILE *logs_file) {
	ParserState p = {
		.lex = lex,
		.logs_file = logs_file
	};
	for (;;) {
		Token t = lex->current_token;
		Lexer_advance(breezeState, lex, logs_file);
		switch (t.type) {
			case TOKEN_ACTION: {
				parseAction(breezeState, &p, t.value, t.line);
				break;
			}
			case TOKEN_DESKTOPS: {
				if (p.flags & PARSERSTATE_HASDESKTOP) {
					registerError(logs_file, "Error: Two desktops scopes defined.\n");
					return;
				}
				p.flags |= PARSERSTATE_HASDESKTOP;
				parseDesktops(breezeState, &p);
				break;
			}
			case TOKEN_BAR: {
				if (p.flags & PARSERSTATE_HASBAR) {
					registerError(logs_file, "Error: Two bar scopes defined.\n");
					return;
				}
				p.flags |= PARSERSTATE_HASBAR;
				parseBar(breezeState, &p);
				break;
			}
			case TOKEN_EOF: {
				return;
			}
			default: {
				registerError(logs_file, "The %s, defined at line %lu, is not a valid top level token.\n", tokenStrings[t.type], t.line);
			}
		}
	}
}

static void parseAction(BreezeState *b, ParserState *s, uptr action, u32 line) {
	Hotkey current_hotkey = {
		.fun = actionMap[action],
		.line = line
	};
	Token t = s->lex->current_token;
	if (t.type != TOKEN_LBRACE) {
		registerError(s->logs_file, "Expected a left brace, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return;
	}
	Lexer_advance(b, s->lex, s->logs_file);

	for (;;) {
		Token t = s->lex->current_token; 
		Lexer_advance(b, s->lex, s->logs_file);
		switch (t.type) {
			case TOKEN_ACTION_ATTRIBUTE: {
				parseActionAttribute(b, s, &current_hotkey, action, t.value);
				break;
			}
			case TOKEN_EOF: {
				registerError(s->logs_file, "Unclosed action scope opened at line %lu.\n", line);
				goto end;
			}
			case TOKEN_RBRACE: {
				goto exit_loop;
			}
			default: {
				registerError(s->logs_file, "Expected an action attribute, got %s at line %lu.\n", tokenStrings[t.type], t.line);
			}
		}
	}
exit_loop:
	if (!likely(current_hotkey.key && current_hotkey.mod && (!actionRequiresArg[action] || current_hotkey.arg))) {
		registerError(s->logs_file, "Not enough attributes defined in the action created at line %lu.\n", current_hotkey.line);
	} else {
		InsertValue(b->hotkeys, current_hotkey, b);
	}
end:
	return;
}

static void parseActionAttribute(BreezeState *b, ParserState *s, Hotkey *hk, uptr action, uptr attr) {
	Token t = s->lex->current_token;
	if (t.type != TOKEN_EQUAL) {
		registerError(s->logs_file, "Expected a = operator, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return;
	}
	Lexer_advance(b, s->lex, s->logs_file);
	t = s->lex->current_token;
	switch (attr) {
		case ATTRIBUTE_KEY: {
			if (t.type == TOKEN_KEY) {
				hk->key = (u32)t.value;
			} else if (t.type == TOKEN_NUMBER && t.value <= 9) {
				hk->key = (u32)(t.value + '0');
			} else if (t.type == TOKEN_POSITION) {
				hk->key = (u32)(t.value + VK_LEFT);
			} else {
				registerError(s->logs_file, "Expected a key token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return;
			}
			Lexer_advance(b, s->lex, s->logs_file);
			if (s->lex->current_token.type == TOKEN_PLUS) {
				registerError(s->logs_file, "You can only assign one key per hotkey. Error at line %lu.\n", t.line);
				Lexer_advance(b, s->lex, s->logs_file);
				return;
			}
			break;
		}
		case ATTRIBUTE_MODIFIER: {
			if (t.type != TOKEN_MODIFIER) {
				registerError(s->logs_file, "Expected a modifier token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return;
			}
			hk->mod = (u32)t.value;
			Lexer_advance(b, s->lex, s->logs_file);
			while (s->lex->current_token.type == TOKEN_PLUS) {
				Lexer_advance(b, s->lex, s->logs_file);
				t = s->lex->current_token;
				if (t.type != TOKEN_MODIFIER) {
					registerError(s->logs_file, "Expected a modifier token after +, got %s at line %lu.\n", tokenStrings[t.type], t.line);
					return;
				}
				Lexer_advance(b, s->lex, s->logs_file);
				hk->mod |= (u32)t.value;
			}
			return;
		}
		case ATTRIBUTE_ARG: {
			Lexer_advance(b, s->lex, s->logs_file);
			if (!actionRequiresArg[action]) {
				registerError(s->logs_file, "The action %s, created at line %lu, musn't have an arg attribute.\n", actionStrings[action], hk->line);
				return;
			}
			if (t.type != TOKEN_STRING) {
				registerError(s->logs_file, "Expected a string token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return;
			}
			hk->arg = (u16 *)t.value;
			return;
		}
	} 
}


static void parseDesktops(BreezeState *b, ParserState *s) {
	DesktopsMods mods = {};
	Token t = s->lex->current_token;
	if (t.type != TOKEN_LBRACE) {
		registerError(s->logs_file, "Expected a left brace, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return;
	}
	u32 scope_line = t.line;
	Lexer_advance(b, s->lex, s->logs_file);

	for (;;) {
		t = s->lex->current_token; 
		switch (t.type) {
			case TOKEN_DESKTOPS_ATTRIBUTE: {
				Lexer_advance(b, s->lex, s->logs_file);
				mods = parseDesktopsAttribute(b, s, mods, t.value);
				break;
			}
			case TOKEN_EOF: {
				registerError(s->logs_file, "Unclosed desktops scope opened at line %lu.\n", tokenStrings[t.type], scope_line);
				goto end;
			}
			case TOKEN_RBRACE: {
				Lexer_advance(b, s->lex, s->logs_file);
				goto exit_loop;
			}
			default: {
				registerError(s->logs_file, "Expected a desktops attribute, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				Lexer_advance(b, s->lex, s->logs_file);
			}
		}
	}
exit_loop:
	if (mods.send == 0 || mods._switch == 0) {
		registerError(s->logs_file, "Attributes not set in desktops scope, created at line %lu.\n", tokenStrings[t.type], scope_line);
		return;
	}
	if (mods.send == mods._switch) {
		registerError(s->logs_file, "Attributes in desktops scope, created at line %lu, have the same value.\n", tokenStrings[t.type], scope_line);
		return;
	}
	for (usize i = 0; i <= 9; ++i) {
		Hotkey switch_to_desktop = {
			.arg = (void *)i,
			.line = scope_line,
			.fun = switchToDesktop,
			.key = (u32)('0' + i),
			.mod = mods._switch
		};
		Hotkey send_to_desktop = {
			.arg = (void *)i,
			.line = scope_line,
			.fun = sendToDesktop,
			.key = (u32)('0' + i),
			.mod = mods.send
		};
		InsertValue(b->hotkeys, switch_to_desktop, b);
		InsertValue(b->hotkeys, send_to_desktop, b);
	}
end:
	return;
}

static DesktopsMods parseDesktopsAttribute(BreezeState *b, ParserState *s, DesktopsMods mods, uptr attr) {
	Token t = s->lex->current_token;
	if (t.type != TOKEN_EQUAL) {
		registerError(s->logs_file, "Expected a = operator, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return mods;
	}
	Lexer_advance(b, s->lex, s->logs_file);
	t = s->lex->current_token;
	switch (attr) {
		case ATTRIBUTE_SEND_TO_DESKTOP: {
			if (t.type != TOKEN_MODIFIER) {
				registerError(s->logs_file, "Expected a modifier token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return mods;
			}
			mods.send |= t.value;
			Lexer_advance(b, s->lex, s->logs_file);
			t = s->lex->current_token;
			while (t.type == TOKEN_PLUS) {
				Lexer_advance(b, s->lex, s->logs_file);
				t = s->lex->current_token;
				if (t.type != TOKEN_MODIFIER) {
					registerError(s->logs_file, "Expected a modifier token after +, got %s at line %lu.\n", tokenStrings[t.type], t.line);
					return mods;
				}
				mods.send |= t.value;
				Lexer_advance(b, s->lex, s->logs_file);
			}
			return mods;
		}
		case ATTRIBUTE_SWITCH_TO_DESKTOP: {
			if (t.type != TOKEN_MODIFIER) {
				registerError(s->logs_file, "Expected a modifier token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return mods;
			}
			mods._switch |= t.value;
			Lexer_advance(b, s->lex, s->logs_file);
			t = s->lex->current_token;
			while (t.type == TOKEN_PLUS) {
				Lexer_advance(b, s->lex, s->logs_file);
				t = s->lex->current_token;
				if (t.type != TOKEN_MODIFIER) {
					registerError(s->logs_file, "Expected a modifier token after +, got %s at line %lu.\n", tokenStrings[t.type], t.line);
					return mods;
				}
				mods._switch |= t.value;
				Lexer_advance(b, s->lex, s->logs_file);
			}
			return mods;
		}
	} 

	return mods;
}


static void parseBar(BreezeState *b, ParserState *s) {
	const u16 *font_str = default_bar_font_str;
	Token t = s->lex->current_token;
	if (t.type != TOKEN_LBRACE) {
		registerError(s->logs_file, "Expected a left brace, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return;
	}
	u32 scope_line = t.line;
	Lexer_advance(b, s->lex, s->logs_file);

	for (;;) {
		t = s->lex->current_token; 
		Lexer_advance(b, s->lex, s->logs_file);
		switch (t.type) {
			case TOKEN_BAR_ATTRIBUTE: {
				font_str = parseBarAttribute(b, s, font_str, t.value);
				break;
			}
			case TOKEN_EOF: {
				registerError(s->logs_file, "Unclosed desktops scope opened at line %lu.\n", scope_line);
				goto end;
			}
			case TOKEN_RBRACE: {
				goto end;
			}
			default: {
				registerError(s->logs_file, "Expected a bar attribute, got %s at line %lu.\n", tokenStrings[t.type], t.line);
			}
		}
	}
end:
	b->bar.current_font = CreateFontW(b->bar.font_height, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, font_str);
	return;
}

static const u16 *parseBarAttribute(BreezeState *b, ParserState *s, const u16 *font_str, uptr attr) {
	Token t = s->lex->current_token;
	if (t.type != TOKEN_EQUAL) {
		registerError(s->logs_file, "Expected a = operator, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return font_str;
	}

	Lexer_advance(b, s->lex, s->logs_file);
	t = s->lex->current_token; 
	Lexer_advance(b, s->lex, s->logs_file);
	switch (attr) {
		case ATTRIBUTE_FOREGROUND: {
			if (t.type != TOKEN_COLOR) {
				registerError(s->logs_file, "Expected a color token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			b->bar.foreground = (COLORREF)t.value;
			return font_str;
		}
		case ATTRIBUTE_BACKGROUND: {
			if (t.type != TOKEN_COLOR) {
				registerError(s->logs_file, "Expected a color token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			b->bar.background = (COLORREF)t.value;
			return font_str;
		}
		case ATTRIBUTE_FONT: {
			if (t.type != TOKEN_STRING) {
				registerError(s->logs_file, "Expected a string token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			return (u16 *)t.value;
		}
		case ATTRIBUTE_FONT_SIZE: {
			if (t.type != TOKEN_NUMBER) {
				registerError(s->logs_file, "Expected a number token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			b->bar.font_height = (DWORD)t.value;
			return font_str;
		}
		case ATTRIBUTE_POSITION: {
			if (t.type != TOKEN_POSITION) {
				registerError(s->logs_file, "Expected a position, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			b->bar.position = (u8)t.value;
			return font_str;
		}
		case ATTRIBUTE_BAR_WIDTH: {
			if (t.type != TOKEN_NUMBER) {
				registerError(s->logs_file, "Expected a number token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			b->bar.width = (u16)t.value;
			return font_str;
		}
		case ATTRIBUTE_BAR_PAD: {
			if (t.type != TOKEN_NUMBER) {
				registerError(s->logs_file, "Expected a number token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			b->bar.padding = (u16)t.value;
			return font_str;
		}
	} 
	return font_str;
}
