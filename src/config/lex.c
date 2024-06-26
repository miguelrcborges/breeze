#include "c.h"

static u8 lread(Lexer *lex) {
	if (lex->pos >= lex->string.len) {
		return '\0';
	}
	return lex->string.str[lex->pos];
}

static bool isAlphanumerical(u8 ch) {
	return ch >= 'a' && ch <= 'z' ||
		ch >= 'A' && ch <= 'Z' ||
		ch >= '0' && ch <= '9' ||
		ch == '_';
}

static void skipIgnore(Lexer *lex) {
	static bool skipTable[(1 << 8) - 1] = {
		['\r'] = 1,
		['\n'] = 1,
		['\t'] = 1,
		[' ']  = 1,
		['\f'] = 1,
		['\v'] = 1,
		[',']  = 1,
		[';']  = 1,
	};

	u8 ch;
	while (skipTable[(ch = lread(lex))]) {
		if (ch == '\n')
			lex->line++;
		lex->pos++;
	}
}

Lexer Lexer_create(string source) {
	return (Lexer) {
		.string = source,
		.pos = 0,
		.line = 1
	};
}

Token Lexer_nextToken(Lexer *lex) {
	skipIgnore(lex);
	u8 ch = lread(lex);
	if (ch == '\'') {
		usize s = lex->pos + 1;
		do {
			lex->pos++;
			ch = lread(lex);
			if (ch == '\n')
				lex->line++;
		} while (ch != '\'' && ch != '\0');
		if (ch == '\0') {
			return (Token) {
				.type = TOKEN_UNTERMINATED_STRING
			};
		}
		string *w = Arena_alloc(&temp, sizeof(string), sizeof(void*));
		w->str = lex->string.str + s;
		w->len = lex->pos - s;
		lex->pos++;
		return (Token) {
			.type = TOKEN_STRING,
			.value = (uptr) w
		};
	} else if (ch == '\0') {
		return (Token) {
			.type = TOKEN_EOF,
			.value = 0 
		};
	}
	string *w = Arena_alloc(&temp, sizeof(string), sizeof(void*));
	usize s = lex->pos;
	w->str = lex->string.str + lex->pos;
	do {
		lex->pos++;
	} while (isAlphanumerical(lread(lex)));
	w->len = lex->pos - s;
	Token t = getToken(w);
	if (t.type == TOKEN_INVALID) {
		io_write(stderr, string_build(&temp, *w, str(" is an invalid token.\n")));
	}
	return t;
}

Token Lexer_peekToken(Lexer *lex) {
	usize pos = lex->pos;
	usize line = lex->line;
	Token t = Lexer_nextToken(lex);
	lex->pos = pos;
	lex->line = line;
	return t;
}
