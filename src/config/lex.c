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
	u8 *p = (u8 *)lex->string.str;
	usize *i = &(lex->pos);
	usize len = lex->string.len;
	
	while (*i < len && 
		(p[*i] == '\n' || p[*i] == '\t' || p[*i] == ' ' || p[*i] == '\t' || p[*i] == '\f' 
		|| p[*i] == ',' || p[*i] == ';')
	) {
		if (p[*i] == '\n')
			lex->line++;
		(*i)++;
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
	if (isAlphanumerical(ch)) {
		string *w = unwrap(Arena_alloc(&temp, sizeof(string), sizeof(void*)));
		usize s = lex->pos;
		w->str = lex->string.str + lex->pos;
		do {
			++lex->pos;
		} while (isAlphanumerical(lread(lex)));
		w->len = lex->pos - s;
		return getToken(w);
	} else if (ch == '\'') {
		usize s = lex->pos;
		do {
			++lex->pos;
			ch = lread(lex);
		} while (ch != '\'' && ch != '\0');
		if (ch == '\0') {
			return (Token) {
				.type = TOKEN_UNTERMINATED_STRING
			};
		}
		string *w = unwrap(Arena_alloc(&temp, sizeof(string), sizeof(void*)));
		w->str = lex->string.str + s;
		w->len = lex->pos - s;
		++lex->pos;
		return (Token) {
			.type = TOKEN_STRING,
			.value = (uptr) w
		};
	}
	string *ts = unwrap(Arena_alloc(&temp, sizeof(string), sizeof(void*)));
	ts->str = lex->string.str + lex->pos;
	ts->len = 1;
	return (Token) {
		.type = TOKEN_INVALID,
		.value = (uptr) ts
	};
}
