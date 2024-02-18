#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "token.h"
#include "types.h"

static bool is_letter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static bool is_space(char c)
{
	return (c == ' ' || (c > 8 && c < 14));
}

static bool is_identifier(char c)
{
	return c == '_' || is_letter(c);
}

static struct Token new_token(struct Lexer *lexer, enum TokenType type)
{
	struct Token token;

	token.type = type;
	if (type == TOKEN_UNKNOWN)
		return token;
	token.literal = lexer->input + lexer->index;
	token.size = lexer->next_index - lexer->index;
	return token;
}

static struct Token new_identifier(struct Lexer *lexer)
{
	struct Token token;

	while (is_identifier(lexer->input[lexer->next_index]))
		++lexer->next_index;

	token.literal = lexer->input + lexer->index;
	token.size = lexer->next_index - lexer->index;
	// TODO: replace with a hashmap ?
	if (token.size == 2 && strncmp(token.literal, "fn", 2) == 0)
		token.type = TOKEN_FUNCTION;
	else if (token.size == 2 && strncmp(token.literal, "if", 2) == 0)
		token.type = TOKEN_IF;
	else if (token.size == 3 && strncmp(token.literal, "let", 3) == 0)
		token.type = TOKEN_LET;
	else if (token.size == 4 && strncmp(token.literal, "true", 4) == 0)
		token.type = TOKEN_TRUE;
	else if (token.size == 4 && strncmp(token.literal, "else", 4) == 0)
		token.type = TOKEN_ELSE;
	else if (token.size == 5 && strncmp(token.literal, "false", 5) == 0)
		token.type = TOKEN_FALSE;
	else if (token.size == 6 && strncmp(token.literal, "return", 6) == 0)
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
	while (is_space(lexer->input[lexer->next_index]))
		++lexer->next_index;
}

bool lexer_init(struct Lexer *lexer, char *filepath)
{
	FILE *file;
	long fsize;

	if ((file = fopen(filepath, "rb")) == NULL)
		return (True);
	fseek(file, 0, SEEK_END);
	if ((fsize = ftell(file)) == -1)
		goto fail_close;
	fseek(file, 0, SEEK_SET);
	lexer->input = malloc(sizeof(char) * (fsize + 1));
	if (lexer->input == NULL)
		goto fail_close;
	fread(lexer->input, fsize, 1, file); // TODO: error check
	fclose(file);
	lexer->input[(size_t)fsize] = '\0';
	lexer->index = 0;
	lexer->next_index = 0;
	return False;
fail_close:
	fclose(file);
	return True;
}

void lexer_destroy(struct Lexer *lexer)
{
	free(lexer->input);
	lexer->input = NULL;
}

struct Token lexer_next_token(struct Lexer *lexer)
{
	skip_whitespaces(lexer);
	lexer->index = lexer->next_index;
	char c = lexer->input[lexer->next_index++];

	switch (c) {
		case '\0':
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
		case '>':
			return new_token(lexer, TOKEN_GREATER);
		case '<':
			return new_token(lexer, TOKEN_LESSER);
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
		default: // identifier, integer, keywords
			if (is_letter(c))
				return new_identifier(lexer);
			if (is_digit(c))
				return new_integer(lexer);
	}
	return new_token(lexer, TOKEN_UNKNOWN);
}

