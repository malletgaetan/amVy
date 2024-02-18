#include "token.h"

char *token_debug[] = 
{
	[TOKEN_EOF] = "EOF",
	[TOKEN_UNKNOWN] = "UNKNOWN",

	[TOKEN_IDENTIFIER] = "IDENTIFIER",
	[TOKEN_INTEGER] = "INTEGER",

	[TOKEN_ASSIGN] = "ASSIGN",
	[TOKEN_PLUS] = "PLUS",
	[TOKEN_MINUS] = "MINUS",
	[TOKEN_ASTERISK] = "ASTERISK",
	[TOKEN_SLASH] = "SLASH",

	[TOKEN_EQUAL] = "EQUAL",
	[TOKEN_NOT_EQUAL] = "NOT EQUAL",
	[TOKEN_LESSER] = "LESS THAN",
	[TOKEN_GREATER] = "GREATER THAN",

	[TOKEN_COMMA] = "COMMA",
	[TOKEN_SEMICOLON] = "SEMICOLON",

	[TOKEN_LPAREN] = "LPAREN",
	[TOKEN_RPAREN] = "RPAREN",
	[TOKEN_LBRACE] = "LBRACE",
	[TOKEN_RBRACE] = "RBRACE",

	[TOKEN_FUNCTION] = "FUNCTION",
	[TOKEN_LET] = "LET",
	[TOKEN_TRUE] = "TRUE", // TODO: maybe get rid of booleans, just like in C
	[TOKEN_FALSE] = "FALSE",
	[TOKEN_IF] = "IF",
	[TOKEN_ELSE] = "ELSE",
	[TOKEN_RETURN] = "RETURN",
};

const char *token_debug_value(enum TokenType type)
{
	return token_debug[type];
}
