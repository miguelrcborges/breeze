#include "../c.h"

typedef u8 TOKEN_TYPE;
enum _TOKEN_TYPE {
	TOKEN_INVALID,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_PLUS,
	TOKEN_ACTION,
	TOKEN_KEY,
	TOKEN_STRING,
	TOKEN_MODIFIER
};

typedef u32 TOKEN_VALUE;
enum TOKEN_ACTION_VALUES {
	ACTION_SPAWN,
	ACTION_RELOAD,
	ACTION_QUIT,
	ACTION_COUNT
};

typedef struct {
	TOKEN_TYPE type;
	TOKEN_VALUE value;
} Token;

typedef struct {
	string string;
	Token token;
} TokenData;


/* map.c */
Token getToken(string s);
