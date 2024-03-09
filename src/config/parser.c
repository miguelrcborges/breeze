#include "c.h"

static HotkeyList htk;
static Lexer *l;
static bool err;

static string tokenStrings[TOKEN_COUNT] = {
	[TOKEN_INVALID] = string("invalid"),
	[TOKEN_EOF] = string("end of file"),
	[TOKEN_UNTERMINATED_STRING] = string("unterminated string"),
	[TOKEN_LBRACE] = string("left brace"),
	[TOKEN_RBRACE] = string("right brace"),
	[TOKEN_PLUS] = string("+ operator"),
	[TOKEN_EQUAL] = string("= operator"),
	[TOKEN_ACTION] = string("action"),
	[TOKEN_ATTRIBUTE] = string("action attribute"),
	[TOKEN_KEY] = string("key token"),
	[TOKEN_STRING] = string("string"),
	[TOKEN_MODIFIER] = string("modifier token")
};

static bool actionRequiresArg[ACTION_COUNT] = {
	[ACTION_SPAWN] = 1,
};

static void (*actionMap[ACTION_COUNT])(void *arg) = {
	[ACTION_SPAWN] = spawn,
	[ACTION_RELOAD] = reloadConfig,
	[ACTION_QUIT] = quit,
};

static void parseAction(uptr action);
static void parseActionAttribute(uptr action, uptr attr, HotkeyList l);

static void htk_insert(HotkeyList node) {
	if (htk == NULL) {
		htk = node;
		node->link = node;
	}
	node->link = htk->link;
	htk->link = node;
}

HotkeyList parse(Lexer *lex) {
	Token t;
	err = 0;
	l = lex;
	htk = NULL;
	for (;;) {
		t = Lexer_nextToken(lex);
		switch (t.type) {
			case TOKEN_ACTION: {
				parseAction(t.value);
				break;
			}
			case TOKEN_UNTERMINATED_STRING: {
				io_write(stderr, string("Unterminated string in the configuration file at an action position.\n"));
				err = 1;
				goto end;
			}
			case TOKEN_EOF: {
				goto end;
			}
			default: {
				string line;
				if (string_fmtu64(&temp, l->line, &line)) {
					line = string("##");
				}
				io_write(stderr, string_build(&temp, string("Expected action, got "), tokenStrings[t.type], string(" at line "), line, string(".\n")));
				err = 1;
			}
		}
	}
end:
	if (err)
		return NULL;
	return htk;
}

static void parseAction(uptr action) {
	Token t = Lexer_nextToken(l);
	if (t.type != TOKEN_LBRACE) {
		string line;
		if (string_fmtu64(&temp, l->line, &line)) {
			line = string("##");
		}
		io_write(stderr, string_build(&temp, string("Expected a left brace, got "), tokenStrings[t.type], string(" at line "), line, string(".\n")));
		err = 1;
		return;
	}

	HotkeyList new = unwrap(Arena_alloc(&temp, sizeof(*new), sizeof(void*)));	
	new->line = l->line;
	new->hk.fun = actionMap[action];
	new->hk.arg = NULL;
	new->mod = 0;
	new->key = 0;
	for (;;) {
		t = Lexer_nextToken(l); 
		switch (t.type) {
			case TOKEN_ATTRIBUTE: {
				parseActionAttribute(action, t.value, new);
				break;
			}
			case TOKEN_EOF: {
				string line;
				if (string_fmtu64(&temp, l->line, &line)) {
					line = string("##");
				}
				io_write(stderr, string_build(&temp, string("Unclosed action scope opened at line "), line, string(".\n")));
				err = 1;
				goto end;
			}
			case TOKEN_RBRACE: {
				goto exit_loop;
			}
			default: {
				string line;
				if (string_fmtu64(&temp, new->line, &line)) {
					line = string("##");
				}
				io_write(stderr, string_build(&temp, string("Expected an action attribute, got "), tokenStrings[t.type], string(" at line "), line, string(".\n")));
				err = 1;
			}
		}
	}
exit_loop:
	if (!likely(new->key && new->mod && (!actionRequiresArg[action] || new->hk.arg))) {
		string line;
		if (string_fmtu64(&temp, new->line, &line)) {
			line = string("##");
		}
		io_write(stderr, string_build(&temp, string("Not enough attributes defined in the action created at line "), line, string(".\n")));
		err = 1;
	} else {
		htk_insert(new);
	}
end:
	return;
}

static void parseActionAttribute(uptr action, uptr attr, HotkeyList hkl) {
	Token t = Lexer_nextToken(l);
	if (t.type != TOKEN_EQUAL) {
		string line;
		err = 1;
		if (string_fmtu64(&temp, l->line, &line)) {
			line = string("##");
		}
		io_write(stderr, string_build(&temp, string("Expected a = operator, got "), tokenStrings[t.type], string(" at line "), line, string(".\n")));
		return;
	}
	t = Lexer_nextToken(l);
	switch (attr) {
		case ATTRIBUTE_KEY: {
			if (t.type != TOKEN_KEY) {
				err = 1;
				string line;
				if (string_fmtu64(&temp, l->line, &line)) {
					line = string("##");
				}
				io_write(stderr, string_build(&temp, string("Inserted a "), tokenStrings[t.type], string(" token in a place of a key token, at line "), line, string(".\n")));
				return;
			}
			u32 tmpkey = t.value;
			if (Lexer_peekToken(l).type == TOKEN_PLUS) {
				err = 1;
				string line;
				if (string_fmtu64(&temp, l->line, &line)) {
					line = string("##");
				}
				io_write(stderr, string_build(&temp, string("You can only assign one key per hotkey. Error at line "), line, string(".\n")));
				return;
			}
			hkl->key = tmpkey;
			return;
		}
		case ATTRIBUTE_MODIFIER: {
			if (t.type != TOKEN_MODIFIER) {
				err = 1;
				string line;
				if (string_fmtu64(&temp, l->line, &line)) {
					line = string("##");
				}
				io_write(stderr, string_build(&temp, string("Expected a modifier token. Got a "), tokenStrings[t.type], string(" at line "), line, string(".\n")));
				return;
			}
			hkl->mod = t.value;
			while (Lexer_peekToken(l).type == TOKEN_PLUS) {
				Lexer_nextToken(l);
				t = Lexer_nextToken(l);
				if (t.type != TOKEN_MODIFIER) {
					err = 1;
					string line;
					if (string_fmtu64(&temp, l->line, &line)) {
						line = string("##");
					}
					io_write(stderr, string_build(&temp, string("Expected a modifier token after +, got "), tokenStrings[t.type], string(" token at line "), line, string(".\n")));
					return;
				}
				hkl->mod |= t.value;
			}
			return;
		}
	} 
}
