#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer/lexer.h"
#include "token/token.h"
#include "libs/string.h"

static char is_letter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static char is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static char is_space(char c)
{
	return (c == ' ' || (c > 8 && c < 14));
}

static char is_identifier(char c)
{
	return c == '_' || is_letter(c);
}

static struct Token new_token(struct Lexer *lexer, enum TokenType type)
{
	struct Token token;

	token.type = type;
	if (type == TOKEN_UNKNOWN)
		return token;
	token.literal.str = lexer->input + lexer->index;
	token.literal.size = lexer->next_index - lexer->index;
	return token;
}

static struct Token new_identifier(struct Lexer *lexer)
{
	struct Token token;

	while (is_identifier(lexer->input[lexer->next_index]))
		++lexer->next_index;

	token.literal.str = lexer->input + lexer->index;
	token.literal.size = lexer->next_index - lexer->index;
	// TODO: replace with a hashmap ?
	if (token.literal.size == 2 && strncmp(token.literal.str, "fn", 2) == 0)
		token.type = TOKEN_FUNCTION;
	else if (token.literal.size == 2 && strncmp(token.literal.str, "if", 2) == 0)
		token.type = TOKEN_IF;
	else if (token.literal.size == 3 && strncmp(token.literal.str, "let", 3) == 0)
		token.type = TOKEN_LET;
	else if (token.literal.size == 4 && strncmp(token.literal.str, "else", 4) == 0)
		token.type = TOKEN_ELSE;
	else if (token.literal.size == 5 && strncmp(token.literal.str, "print", 5) == 0)
		token.type = TOKEN_PRINT;
	else if (token.literal.size == 5 && strncmp(token.literal.str, "while", 5) == 0)
		token.type = TOKEN_WHILE;
	else if (token.literal.size == 6 && strncmp(token.literal.str, "return", 6) == 0)
		token.type = TOKEN_RETURN;
	else
		token.type = TOKEN_IDENTIFIER;
	return token;
}

static struct Token new_integer(struct Lexer *lexer)
{
	while (is_digit(lexer->input[lexer->next_index]))
		++lexer->next_index;
	return new_token(lexer, TOKEN_INTEGER);
}

static void skip_whitespaces(struct Lexer *lexer)
{
	while (lexer->input[lexer->index] && is_space(lexer->input[lexer->next_index]))
	{
		if (lexer->input[lexer->next_index] == '\n')
			++lexer->line;
		++lexer->next_index;
	}
}

void lexer_init(struct Lexer *lexer, char *file_content)
{
	lexer->input = file_content;
	lexer->index = 0;
	lexer->next_index = 0;
	lexer->line = 1;
}

void lexer_destroy(struct Lexer *lexer)
{
	free(lexer->input);
	lexer->input = NULL;
}

void lexer_zero_string(struct Lexer *lexer)
{
	size_t i = 0;
	int last_identifier = 0;
	while (lexer->input[i])
	{
		if (last_identifier == 1 && !is_identifier(lexer->input[i])) {
			lexer->input[i] = '\0';
			last_identifier = 0;
		}
		else if (is_identifier(lexer->input[i]))
			last_identifier = 1;
		++i;
	}
}

struct Token lexer_next_token(struct Lexer *lexer)
{
	skip_whitespaces(lexer);
	lexer->index = lexer->next_index;
	if (!lexer->input[lexer->index])
			return new_token(lexer, TOKEN_EOF);
	char c = lexer->input[lexer->next_index++];

	switch (c) {
		case '\0': // NOTE: this case should remain first
			return new_token(lexer, TOKEN_EOF);
		case '=':
			if (lexer->input[lexer->next_index] == '=')
			{
				++lexer->next_index;
				return new_token(lexer, TOKEN_EQUAL);
			}
			return new_token(lexer, TOKEN_ASSIGN);
		case '!':
			if (lexer->input[lexer->next_index] != '=')
				return new_token(lexer, TOKEN_UNKNOWN);
			++lexer->next_index;
			return new_token(lexer, TOKEN_NOT_EQUAL);
		case '*':
			return new_token(lexer, TOKEN_ASTERISK);
		case '/':
			return new_token(lexer, TOKEN_SLASH);
		case '%':
			return new_token(lexer, TOKEN_MODULO);
		case '>':
			return new_token(lexer, TOKEN_GREATER);
		case '<':
			return new_token(lexer, TOKEN_LESSER);
		case '-':
			return new_token(lexer, TOKEN_MINUS);
		case '+':
			return new_token(lexer, TOKEN_PLUS);
		case ',':
			return new_token(lexer, TOKEN_COMMA);
		case ';':
			return new_token(lexer, TOKEN_SEMICOLON);
		case '(':
			return new_token(lexer, TOKEN_LPAREN);
		case ')':
			return new_token(lexer, TOKEN_RPAREN);
		case '{':
			return new_token(lexer, TOKEN_LBRACE);
		case '}':
			return new_token(lexer, TOKEN_RBRACE);
		case '[':
			return new_token(lexer, TOKEN_LBRACKET);
		case ']':
			return new_token(lexer, TOKEN_RBRACKET);
		default: // identifier, integer, keywords
			if (is_letter(c))
				return new_identifier(lexer);
			if (is_digit(c))
				return new_integer(lexer);
	}
	return new_token(lexer, TOKEN_UNKNOWN);
}

