#ifndef TOKEN_H
# define TOKEN_H

# include "types.h"

enum TokenType {
	TOKEN_EOF,
	TOKEN_UNKNOWN,

	TOKEN_IDENTIFIER,
	TOKEN_INTEGER,

	TOKEN_ASSIGN,
	TOKEN_PLUS,

	TOKEN_COMMA,
	TOKEN_SEMICOLON,

	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,

	TOKEN_FUNCTION,
	TOKEN_LET,
};

struct Token {
	enum TokenType type;
	const char *literal;
	u16 size;
};

const char *token_debug_value(enum TokenType type);

#endif
