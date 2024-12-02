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

static void (*actionMap[ACTION_COUNT])(void *arg) = {
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


static void parseAction(ParserState *s, uptr action, u32 line);
static void parseActionAttribute(ParserState *s, uptr action, uptr attr);
static void parseDesktops(ParserState *s);
static DesktopsMods parseDesktopsAttribute(ParserState *s, DesktopsMods mods, uptr attr);
static void parseBar(ParserState *s);
static const u16 *parseBarAttribute(ParserState *s, const u16 *font_str, uptr attr);

void parse(Lexer *lex, FILE *logs_file) {
	ParserState p = {
		.lex = lex,
		.logs_file = logs_file
	};
	for (;;) {
		Token t = lex->current_token;
		Lexer_advance(lex, logs_file);
		switch (t.type) {
			case TOKEN_ACTION: {
				parseAction(&p, t.value, t.line);
				break;
			}
			case TOKEN_DESKTOPS: {
				if (p.flags & PARSERSTATE_HASDESKTOP) {
					registerError(logs_file, "Error: Two desktops scopes defined.\n");
					return;
				}
				p.flags |= PARSERSTATE_HASDESKTOP;
				parseDesktops(&p);
				break;
			}
			case TOKEN_BAR: {
				if (p.flags & PARSERSTATE_HASBAR) {
					registerError(logs_file, "Error: Two bar scopes defined.\n");
					return;
				}
				p.flags |= PARSERSTATE_HASBAR;
				parseBar(&p);
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

static void parseAction(ParserState *s, uptr action, u32 line) {
	hotkeys_buf[hotkeys_count] = (Hotkey){
		.fun = actionMap[action], 
		.line = line
	};
	Token t = s->lex->current_token;
	if (t.type != TOKEN_LBRACE) {
		registerError(s->logs_file, "Expected a left brace, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return;
	}
	Lexer_advance(s->lex, s->logs_file);

	for (;;) {
		Token t = s->lex->current_token; 
		Lexer_advance(s->lex, s->logs_file);
		switch (t.type) {
			case TOKEN_ACTION_ATTRIBUTE: {
				parseActionAttribute(s, action, t.value);
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
	if (!likely(hotkeys_buf[hotkeys_count].key && hotkeys_buf[hotkeys_count].mod && (!actionRequiresArg[action] || hotkeys_buf[hotkeys_count].arg))) {
		registerError(s->logs_file, "Not enough attributes defined in the action created at line %lu.\n", hotkeys_buf[hotkeys_count].line);

	} else {
		hotkeys_count += 1;
	}
end:
	return;
}

static void parseActionAttribute(ParserState *s, uptr action, uptr attr) {
	Token t = s->lex->current_token;
	if (t.type != TOKEN_EQUAL) {
		registerError(s->logs_file, "Expected a = operator, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return;
	}
	Lexer_advance(s->lex, s->logs_file);
	t = s->lex->current_token;
	switch (attr) {
		case ATTRIBUTE_KEY: {
			if (t.type == TOKEN_KEY) {
				hotkeys_buf[hotkeys_count].key = (u32)t.value;
			} else if (t.type == TOKEN_NUMBER && t.value <= 9) {
				hotkeys_buf[hotkeys_count].key = (u32)(t.value + '0');
			} else if (t.type == TOKEN_POSITION) {
				hotkeys_buf[hotkeys_count].key = (u32)(t.value + VK_LEFT);
			} else {
				registerError(s->logs_file, "Expected a key token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return;
			}
			Lexer_advance(s->lex, s->logs_file);
			if (s->lex->current_token.type == TOKEN_PLUS) {
				registerError(s->logs_file, "You can only assign one key per hotkey. Error at line %lu.\n", t.line);
				Lexer_advance(s->lex, s->logs_file);
				return;
			}
			break;
		}
		case ATTRIBUTE_MODIFIER: {
			if (t.type != TOKEN_MODIFIER) {
				registerError(s->logs_file, "Expected a modifier token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return;
			}
			hotkeys_buf[hotkeys_count].mod = (u32)t.value;
			Lexer_advance(s->lex, s->logs_file);
			while (s->lex->current_token.type == TOKEN_PLUS) {
				Lexer_advance(s->lex, s->logs_file);
				t = s->lex->current_token;
				if (t.type != TOKEN_MODIFIER) {
					registerError(s->logs_file, "Expected a modifier token after +, got %s at line %lu.\n", tokenStrings[t.type], t.line);
					return;
				}
				Lexer_advance(s->lex, s->logs_file);
				hotkeys_buf[hotkeys_count].mod |= (u32)t.value;
			}
			return;
		}
		case ATTRIBUTE_ARG: {
			Lexer_advance(s->lex, s->logs_file);
			if (!actionRequiresArg[action]) {
				registerError(s->logs_file, "The action %s, created at line %lu, musn't have an arg attribute.\n", actionStrings[action], hotkeys_buf[hotkeys_count].line);
				return;
			}
			if (t.type != TOKEN_STRING) {
				registerError(s->logs_file, "Expected a string token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return;
			}
			hotkeys_buf[hotkeys_count].arg = (u16 *)t.value;
			return;
		}
	} 
}


static void parseDesktops(ParserState *s) {
	DesktopsMods mods = {};
	Token t = s->lex->current_token;
	if (t.type != TOKEN_LBRACE) {
		registerError(s->logs_file, "Expected a left brace, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return;
	}
	u32 scope_line = t.line;
	Lexer_advance(s->lex, s->logs_file);

	for (;;) {
		t = s->lex->current_token; 
		switch (t.type) {
			case TOKEN_DESKTOPS_ATTRIBUTE: {
				Lexer_advance(s->lex, s->logs_file);
				mods = parseDesktopsAttribute(s, mods, t.value);
				break;
			}
			case TOKEN_EOF: {
				registerError(s->logs_file, "Unclosed desktops scope opened at line %lu.\n", tokenStrings[t.type], scope_line);
				goto end;
			}
			case TOKEN_RBRACE: {
				Lexer_advance(s->lex, s->logs_file);
				goto exit_loop;
			}
			default: {
				registerError(s->logs_file, "Expected a desktops attribute, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				Lexer_advance(s->lex, s->logs_file);
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
		hotkeys_buf[hotkeys_count++] = (Hotkey) {
			.arg = (void *)i,
			.line = scope_line,
			.fun = switchToDesktop,
			.key = (u32)('0' + i),
			.mod = mods._switch
		};
		hotkeys_buf[hotkeys_count++] = (Hotkey) {
			.arg = (void *)i,
			.line = scope_line,
			.fun = sendToDesktop,
			.key = (u32)('0' + i),
			.mod = mods.send
		};
	}
end:
	return;
}

static DesktopsMods parseDesktopsAttribute(ParserState *s, DesktopsMods mods, uptr attr) {
	Token t = s->lex->current_token;
	if (t.type != TOKEN_EQUAL) {
		registerError(s->logs_file, "Expected a = operator, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return mods;
	}
	Lexer_advance(s->lex, s->logs_file);
	t = s->lex->current_token;
	switch (attr) {
		case ATTRIBUTE_SEND_TO_DESKTOP: {
			if (t.type != TOKEN_MODIFIER) {
				registerError(s->logs_file, "Expected a modifier token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return mods;
			}
			mods.send |= t.value;
			Lexer_advance(s->lex, s->logs_file);
			t = s->lex->current_token;
			while (t.type == TOKEN_PLUS) {
				Lexer_advance(s->lex, s->logs_file);
				t = s->lex->current_token;
				if (t.type != TOKEN_MODIFIER) {
					registerError(s->logs_file, "Expected a modifier token after +, got %s at line %lu.\n", tokenStrings[t.type], t.line);
					return mods;
				}
				mods.send |= t.value;
				Lexer_advance(s->lex, s->logs_file);
			}
			return mods;
		}
		case ATTRIBUTE_SWITCH_TO_DESKTOP: {
			if (t.type != TOKEN_MODIFIER) {
				registerError(s->logs_file, "Expected a modifier token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return mods;
			}
			mods._switch |= t.value;
			Lexer_advance(s->lex, s->logs_file);
			t = s->lex->current_token;
			while (t.type == TOKEN_PLUS) {
				Lexer_advance(s->lex, s->logs_file);
				t = s->lex->current_token;
				if (t.type != TOKEN_MODIFIER) {
					registerError(s->logs_file, "Expected a modifier token after +, got %s at line %lu.\n", tokenStrings[t.type], t.line);
					return mods;
				}
				mods._switch |= t.value;
				Lexer_advance(s->lex, s->logs_file);
			}
			return mods;
		}
	} 

	return mods;
}


static void parseBar(ParserState *s) {
	const u16 *font_str = default_bar_font_str;
	Token t = s->lex->current_token;
	if (t.type != TOKEN_LBRACE) {
		registerError(s->logs_file, "Expected a left brace, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return;
	}
	u32 scope_line = t.line;
	Lexer_advance(s->lex, s->logs_file);

	for (;;) {
		t = s->lex->current_token; 
		Lexer_advance(s->lex, s->logs_file);
		switch (t.type) {
			case TOKEN_BAR_ATTRIBUTE: {
				font_str = parseBarAttribute(s, font_str, t.value);
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
	bar_font = CreateFontW(bar_font_height, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, font_str);
	return;
}

static const u16 *parseBarAttribute(ParserState *s, const u16 *font_str, uptr attr) {
	Token t = s->lex->current_token;
	if (t.type != TOKEN_EQUAL) {
		registerError(s->logs_file, "Expected a = operator, got %s at line %lu.\n", tokenStrings[t.type], t.line);
		return font_str;
	}

	Lexer_advance(s->lex, s->logs_file);
	t = s->lex->current_token; 
	Lexer_advance(s->lex, s->logs_file);
	switch (attr) {
		case ATTRIBUTE_FOREGROUND: {
			if (t.type != TOKEN_COLOR) {
				registerError(s->logs_file, "Expected a color token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			foreground = (COLORREF)t.value;
			return font_str;
		}
		case ATTRIBUTE_BACKGROUND: {
			if (t.type != TOKEN_COLOR) {
				registerError(s->logs_file, "Expected a color token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			background = (COLORREF)t.value;
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
			bar_font_height = (DWORD)t.value;
			return font_str;
		}
		case ATTRIBUTE_POSITION: {
			if (t.type != TOKEN_POSITION) {
				registerError(s->logs_file, "Expected a position, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			bar_position = (int)t.value;
			return font_str;
		}
		case ATTRIBUTE_BAR_WIDTH: {
			if (t.type != TOKEN_NUMBER) {
				registerError(s->logs_file, "Expected a number token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			bar_width = (int)t.value;
			return font_str;
		}
		case ATTRIBUTE_BAR_PAD: {
			if (t.type != TOKEN_NUMBER) {
				registerError(s->logs_file, "Expected a number token, got %s at line %lu.\n", tokenStrings[t.type], t.line);
				return font_str;
			}
			bar_pad = (int)t.value;
			return font_str;
		}
	} 
	return font_str;
}
