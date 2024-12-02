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
	TOKEN_BAR,
	TOKEN_BAR_ATTRIBUTE,
	TOKEN_COLOR,
	TOKEN_INVALID_COLOR,
	TOKEN_NUMBER,
	TOKEN_POSITION,
	TOKEN_COUNT
};

enum TOKEN_ACTION_VALUES {
	ACTION_SPAWN,
	ACTION_COMMAND_LINE,
	ACTION_RELOAD,
	ACTION_QUIT,
	ACTION_KILL,
	ACTION_FOCUS_NEXT,
	ACTION_FOCUS_PREV,
	ACTION_REVEAL_ALL,
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

enum TOKEN_BAR_ATTRIBUTES {
	ATTRIBUTE_BACKGROUND,
	ATTRIBUTE_FOREGROUND,
	ATTRIBUTE_FONT,
	ATTRIBUTE_FONT_SIZE,
	ATTRIBUTE_POSITION,
	ATTRIBUTE_BAR_WIDTH,
	ATTRIBUTE_BAR_PAD,
};

typedef struct {
	CONFIG_TOKEN_TYPE type;
	u32 line;
	uptr value;
} Token;

typedef struct {
	char *string;
	Token token;
} TokenData;

typedef struct {
	char *source;
	usize pos;
	usize line;
	Token current_token;
} Lexer;

typedef struct {
	u16 buffer[WIDESTR_ALLOC_BUFF_SIZE];
	u16 position;
} WidestringAllocator;

/* config.c */
void registerError(FILE *logs_file, const char *error_fmt, ...);

/* lex.c */
Lexer Lexer_create(char *source, FILE *logs_file);
void Lexer_advance(Lexer *lex, FILE *logs_file);
extern WidestringAllocator widestringAllocator;

/* map.c */
Token getToken(char *s);

/* parser.c */
void parse(Lexer *lex, FILE *logs_file);

#endif
