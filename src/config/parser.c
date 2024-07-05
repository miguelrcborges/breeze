#include "c.h"

#include "stdio.h"

static Lexer *l;
static bool err;

static char *tokenStrings[TOKEN_COUNT] = {
	[TOKEN_INVALID] = "invalid",
	[TOKEN_EOF] = "end of file",
	[TOKEN_UNTERMINATED_STRING] = "unterminated string",
	[TOKEN_LBRACE] = "left brace",
	[TOKEN_RBRACE] = "right brace",
	[TOKEN_PLUS] = "+ operator",
	[TOKEN_EQUAL] = "= operator",
	[TOKEN_ACTION] = "action",
	[TOKEN_ATTRIBUTE] = "action attribute",
	[TOKEN_KEY] = "key token",
	[TOKEN_STRING] = "string",
	[TOKEN_MODIFIER] = "modifier token"
};

static bool actionRequiresArg[ACTION_COUNT] = {
	[ACTION_SPAWN] = 1,
};

static void (*actionMap[ACTION_COUNT])(void *arg) = {
	[ACTION_SPAWN] = spawn,
	[ACTION_RELOAD] = reloadConfig,
	[ACTION_QUIT] = quit,
	[ACTION_KILL] = kill,
};

static char *actionStrings[ACTION_COUNT] = {
	[ACTION_SPAWN] = "spawn",
	[ACTION_RELOAD] = "reload",
	[ACTION_QUIT] = "quit",
	[ACTION_KILL] = "kill",
};

static void parseAction(uptr action);
static void parseActionAttribute(uptr action, uptr attr);

bool parse(Lexer *lex) {
	Token t;
	err = 0;
	l = lex;
	for (;;) {
		t = Lexer_nextToken(lex);
		switch (t.type) {
			case TOKEN_ACTION: {
				parseAction(t.value);
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
			case TOKEN_ATTRIBUTE: {
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
				fprintf(stderr, "Expected a string, got %s at line %llu.\n", tokenStrings[t.type], l->line);
				return;
			}
			hotkeys_buf[hotkeys_count].arg = (u16 *)t.value;
			return;
		}
	} 
}
