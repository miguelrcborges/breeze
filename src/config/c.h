#ifndef CONFIG_C_H
#define CONFIG_C_H

#include "../c.h"

typedef u8 CONFIG_TOKEN_TYPE;
enum _CONFIG_TOKEN_TYPE {
	TOKEN_INVALID,
	TOKEN_EOF,
	TOKEN_UNTERMINATED_STRING,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_PLUS,
	TOKEN_EQUAL,
	TOKEN_ACTION,
	TOKEN_ACTION_ATTRIBUTE,
	TOKEN_KEY,
	TOKEN_MODIFIER,
	TOKEN_STRING,
	TOKEN_DESKTOPS,
	TOKEN_DESKTOPS_ATTRIBUTE,
	TOKEN_COUNT
};

enum TOKEN_ACTION_VALUES {
	ACTION_SPAWN,
	ACTION_RELOAD,
	ACTION_QUIT,
	ACTION_KILL,
	ACTION_COUNT
};

enum TOKEN_ACTION_ATTRIBUTES {
	ATTRIBUTE_KEY,
	ATTRIBUTE_MODIFIER,
	ATTRIBUTE_ARG
};

enum TOKEN_DESKTOPS_ATTRIBUTES {
	ATTRIBUTE_SEND_TO_DESKTOP,
	ATTRIBUTE_SWITCH_TO_DESKTOP,
};

typedef struct {
	CONFIG_TOKEN_TYPE type;
	uptr value;
} Token;

typedef struct {
	char *string;
	Token token;
} TokenData;

typedef struct {
	char *string;
	usize pos;
	usize line;
	usize alloc_pos;
} Lexer;

/* lex.c */
Lexer Lexer_create(char *source);
Token Lexer_nextToken(Lexer *lex);
Token Lexer_peekToken(Lexer *lex);

/* map.c */
Token getToken(char *s);

/* parser.c */
bool parse(Lexer *lex);

#endif
