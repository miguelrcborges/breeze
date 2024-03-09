#include "../c.h"

typedef u8 TOKEN_TYPE;
enum _TOKEN_TYPE {
	TOKEN_INVALID,
	TOKEN_EOF,
	TOKEN_UNTERMINATED_STRING,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_PLUS,
	TOKEN_EQUAL,
	TOKEN_ACTION,
	TOKEN_ATTRIBUTE,
	TOKEN_KEY,
	TOKEN_MODIFIER,
	TOKEN_STRING,
	TOKEN_COUNT
};

typedef uptr TOKEN_VALUE;
enum TOKEN_ACTION_VALUES {
	ACTION_SPAWN,
	ACTION_RELOAD,
	ACTION_QUIT,
	ACTION_COUNT
};

typedef uptr ATTRIBUTES;
enum TOKEN_ACTION_ATTRIBUTES {
	ATTRIBUTE_KEY,
	ATTRIBUTE_MODIFIER,
	ATTRIBUTE_ARG
};

typedef struct {
	TOKEN_TYPE type;
	TOKEN_VALUE value;
} Token;

typedef struct {
	string string;
	Token token;
} TokenData;

typedef struct {
	string string;
	usize pos;
	usize line;
} Lexer;

typedef struct hotkeyList *HotkeyList;
struct hotkeyList {
	Hotkey hk;
	HotkeyList link;
	usize line;
	u32 key;
	u32 mod;
};

/* lex.c */
Lexer Lexer_create(string source);
Token Lexer_nextToken(Lexer *lex);
Token Lexer_peekToken(Lexer *lex);

/* map.c */
Token getToken(string *s);

/* parser.c */
HotkeyList parse(Lexer *lex);
