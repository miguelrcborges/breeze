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
	[TOKEN_ACTION] = string("action"),
	[TOKEN_KEY] = string("key token"),
	[TOKEN_STRING] = string("string"),
	[TOKEN_MODIFIER] = string("modifier token")
};

static void parseAction(Token actionToken);

HotkeyList parse(Lexer *lex) {
	Token t;
	err = 0;
	l = lex;
	htk = NULL;
	for (;;) {
		t = Lexer_nextToken(lex);
		switch (t.type) {
			case TOKEN_ACTION: {
				parseAction(t);
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

static void parseAction(Token actionToken) {
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

	usize start_line = l->line;
	HotkeyList new = unwrap(Arena_alloc(&temp, sizeof(HotkeyList), sizeof(void*)));	
	for (;;) {
		t = Lexer_nextToken(l); 
		switch (t.type) {
			case TOKEN_EOF: {
				string line;
				if (string_fmtu64(&temp, start_line, &line)) {
					line = string("##");
				}
				io_write(stderr, string_build(&temp, string("Unclosed action scope opened at line "), line, string(".\n")));
				err = 1;
				goto end;
			}
			case TOKEN_RBRACE: {
				goto exit_loop;
			}
		}
	}
exit_loop:
end:
	return;
}
