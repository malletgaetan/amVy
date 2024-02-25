#ifndef TOKEN_H
# define TOKEN_H

# include "libs/types.h"
# include "libs/string.h"

enum TokenType {
	TOKEN_EOF,
	TOKEN_UNKNOWN,

	TOKEN_IDENTIFIER,
	TOKEN_INTEGER,

	TOKEN_ASSIGN,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_ASTERISK,
	TOKEN_SLASH,
	TOKEN_MODULO,

	TOKEN_EQUAL,
	TOKEN_NOT_EQUAL,
	TOKEN_LESSER,
	TOKEN_GREATER,

	TOKEN_COMMA,
	TOKEN_SEMICOLON,

	TOKEN_LBRACKET,
	TOKEN_RBRACKET,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,

	// keywords
	TOKEN_FUNCTION,
	TOKEN_LET,
	TOKEN_TRUE, // TODO: maybe get rid of booleans, just like in C
	TOKEN_FALSE,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_RETURN,
	TOKEN_LIMIT,
};

struct Token {
	enum TokenType type;
	struct String literal;
};

const char *token_debug_value(enum TokenType type);

#endif
