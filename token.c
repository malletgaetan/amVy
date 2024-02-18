#include "token.h"

char *token_debug[] = 
{
	[TOKEN_EOF] = "EOF",
	[TOKEN_UNKNOWN] = "UNKNOWN",

	[TOKEN_IDENTIFIER] = "IDENTIFIER",
	[TOKEN_INTEGER] = "INTEGER",

	[TOKEN_ASSIGN] = "ASSIGN",
	[TOKEN_PLUS] = "PLUS",

	[TOKEN_COMMA] = "COMMA",
	[TOKEN_SEMICOLON] = "SEMICOLON",

	[TOKEN_LPAREN] = "LPAREN",
	[TOKEN_RPAREN] = "RPAREN",
	[TOKEN_LBRACE] = "LBRACE",
	[TOKEN_RBRACE] = "RBRACE",

	[TOKEN_FUNCTION] = "FUNCTION",
	[TOKEN_LET] = "LET",
};

const char *token_debug_value(enum TokenType type)
{
	return token_debug[type];
}
