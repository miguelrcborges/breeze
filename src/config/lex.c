#include "c.h"

#include <stdio.h>

WidestringAllocator widestringAllocator;

static bool isAlphanumerical(u8 ch) {
	return ch >= 'a' && ch <= 'z' ||
		ch >= 'A' && ch <= 'Z' ||
		ch >= '0' && ch <= '9' ||
		ch == '_';
}

static bool isValidColorComponent(u8 ch) {
	return ch >= 'a' && ch <= 'f' ||
		ch >= 'A' && ch <= 'F' ||
		ch >= '0' && ch <= '9';
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
	while (skipTable[ch = lex->source[lex->pos]]) {
		if (ch == '\n')
			lex->line++;
		lex->pos++;
	}
}

Lexer Lexer_create(char *source) {
	return (Lexer) {
		.source = source,
		.pos = 0,
		.line = 1,
	};
}

Token Lexer_nextToken(Lexer *lex) {
	skipIgnore(lex);
	u8 ch = lex->source[lex->pos];
	if (ch == '\'') {
		usize s = lex->pos + 1;
		do {
			lex->pos++;
			ch = lex->source[lex->pos];
			if (ch == '\n')
				lex->line++;
		} while (ch != '\'' && ch != '\0');
		if (ch == '\0') {
			return (Token) {
				.type = TOKEN_UNTERMINATED_STRING
			};
		}
		u16 *wstr = widestringAllocator.buffer + widestringAllocator.position;
		int written = MultiByteToWideChar(CP_UTF8, 0, lex->source + s, lex->pos - s, wstr, len(widestringAllocator.buffer) - widestringAllocator.position);
		if (unlikely(written == 0)) {
			fprintf(stderr, "Failed to convert UTF-8 string to UTF-16: %lu.\n", GetLastError());
			exit(1);
		}
		wstr[written] = '\0';
		lex->pos++;
		widestringAllocator.position += written + 1;
		return (Token) {
			.type = TOKEN_STRING,
			.value = (uptr) wstr
		};
	} else if (ch == '\0') {
		return (Token) {
			.type = TOKEN_EOF,
			.value = 0 
		};
	} else if (ch == '#') {
		uptr color = 0;
		usize count = 0;
		for (usize count = 0; count < 6; ++count) {
			lex->pos++;
			u8 ch = lex->source[lex->pos];
			if (!isValidColorComponent(ch)) {
				return (Token) {
					.type = TOKEN_INVALID_COLOR,
					.value = 0
				};
			}
			color <<= 4;
			if (ch >= 'a' && ch <= 'f')
				color |= ch - 'a' + 10;
			else if (ch >= 'A' && ch <= 'F')
				color |= ch - 'A' + 10;
			else
				color |= ch - '0';
		}
		lex->pos++;
		// convert rrggbb to bbggrr
		color = ((color & 0xff0000) >> 16) | (color & 0x00ff00) | ((color & 0x0000ff) << 16);
		return (Token) {
			.type = TOKEN_COLOR,
			.value = color
		};
	} else if (ch >= '0' && ch <= '9') {
		uptr num = ch - '0';
		lex->pos++;
		u8 ch = lex->source[lex->pos];
		while (ch >= '0' && ch <= '9') {
			num *= 10;
			num += ch - '0';
			lex->pos++;
			ch = lex->source[lex->pos];
		}
		return (Token) {
			.type = TOKEN_NUMBER,
			.value = num
		};
	}
	usize s = lex->pos;
	do {
		lex->pos++;
	} while (isAlphanumerical(lex->source[lex->pos]));
	u8 tmp = lex->source[lex->pos];
	lex->source[lex->pos] = '\0';
	Token t = getToken(lex->source + s);
	lex->source[lex->pos] = tmp;
	if (unlikely(t.type == TOKEN_INVALID)) {
		fprintf(stderr, "The token \"%*.s\", at line %llu, is invalid.\n", (int)(lex->pos - s - 1), lex->source + s, lex->line);
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
