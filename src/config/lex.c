#include "c.h"

#include <stdio.h>

WidestringAllocator widestringAllocator;

static bool isAlphanumerical(u8 ch) {
	return (ch >= 'a' && ch <= 'z') ||
		(ch >= 'A' && ch <= 'Z') ||
		(ch >= '0' && ch <= '9') ||
		(ch == '_');
}

static bool isValidColorComponent(u8 ch) {
	return (ch >= 'a' && ch <= 'f') ||
		(ch >= 'A' && ch <= 'F') ||
		(ch >= '0' && ch <= '9');
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

Lexer Lexer_create(char *source, FILE *logs_file) {
	Lexer lex = {
		.source = source,
		.pos = 0,
		.line = 1,
	};
	Lexer_advance(&lex, logs_file);
	return lex;
}

void Lexer_advance(Lexer *lex, FILE *logs_file) {
	skipIgnore(lex);
	u32 line = lex->line;
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
			lex->current_token = (Token) {
				.type = TOKEN_UNTERMINATED_STRING,
				.line = line
			};
			return;
		}
		u16 *wstr = widestringAllocator.buffer + widestringAllocator.position;
		u16 written = (u16)MultiByteToWideChar(CP_UTF8, 0, lex->source + s, (int)(lex->pos - s), wstr, (int)(len(widestringAllocator.buffer) - widestringAllocator.position));
		if (unlikely(written == 0)) {
			fprintf(stderr, "Failed to convert UTF-8 string to UTF-16: %lu.\n", GetLastError());
			exit(1);
		}
		wstr[written] = '\0';
		lex->pos++;
		widestringAllocator.position += written + 1;
		lex->current_token = (Token) {
			.type = TOKEN_STRING,
			.line = line,
			.value = (uptr) wstr
		};
		return;

	} else if (ch == '\0') {
		lex->current_token = (Token) {
			.type = TOKEN_EOF,
			.line = line,
			.value = 0
		};
		return;

	} else if (ch == '#') {
		uptr color = 0;
		for (usize count = 0; count < 6; ++count) {
			lex->pos++;
			u8 ch = lex->source[lex->pos];
			if (!isValidColorComponent(ch)) {
				lex->current_token = (Token) {
					.type = TOKEN_INVALID_COLOR,
					.line = line,
					.value = 0
				};
				return;
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
		lex->current_token = (Token) {
			.type = TOKEN_COLOR,
			.line = line,
			.value = color
		};
		return;

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

		lex->current_token = (Token) {
			.type = TOKEN_NUMBER,
			.line = line,
			.value = num
		};
		return;
	}
	usize s = lex->pos;
	do {
		lex->pos++;
	} while (isAlphanumerical(lex->source[lex->pos]));
	u8 tmp = lex->source[lex->pos];
	lex->source[lex->pos] = '\0';
	lex->current_token = getToken(lex->source + s);
	lex->current_token.line = line;
	if (unlikely(lex->current_token.type == TOKEN_INVALID)) {
		registerError(logs_file, "The token \"%s\", at line %lu, is invalid.\n", lex->source + s, line);
	}
	lex->source[lex->pos] = tmp;
	return;
}

Token Lexer_peekToken(Lexer *lex) {
	return lex->current_token;
}
