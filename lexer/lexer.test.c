#include <assert.h>
#include <stdio.h>

#include "lexer/lexer.h"
#include "token/token.h"

struct TestCase {
	char input[50];
	enum TokenType expected[50];
	enum TokenType got[50];
};

void print_tokens(enum TokenType *tokens, unsigned int size)
{
	for (unsigned int i = 0; i < size; i++)
		printf("%s\n", token_debug_value(tokens[i]));
}

void print_bad_case(struct TestCase testCase, unsigned int i)
{
	printf("lexer_tests: \"%s\" FAILED\n", testCase.input);
	printf("---- EXPECTED %u---\n", i);
	print_tokens(testCase.expected, i + 1);
	printf("------ GOT ------\n");
	print_tokens(testCase.got, i + 1);
}

int main(void)
{
	struct TestCase tests[] = {
		{
			"let a = 1;",
			{
				TOKEN_LET,
				TOKEN_IDENTIFIER,
				TOKEN_ASSIGN,
				TOKEN_INTEGER,
				TOKEN_SEMICOLON,
				TOKEN_EOF,
			},
			{0},
		},
		{
			"(1+3)*5;",
			{
				TOKEN_LPAREN,
				TOKEN_INTEGER,
				TOKEN_PLUS,
				TOKEN_INTEGER,
				TOKEN_RPAREN,
				TOKEN_ASTERISK,
				TOKEN_INTEGER,
				TOKEN_SEMICOLON,
				TOKEN_EOF,
			},
			{0},
		},
		{
			"letfntruefalse",
			{
				TOKEN_IDENTIFIER,
				TOKEN_EOF,
			},
			{0},
		},
		{
			"fn (a,b,c,d) { return dang }",
			{
				TOKEN_FUNCTION,
				TOKEN_LPAREN,
				TOKEN_IDENTIFIER,
				TOKEN_COMMA,
				TOKEN_IDENTIFIER,
				TOKEN_COMMA,
				TOKEN_IDENTIFIER,
				TOKEN_COMMA,
				TOKEN_IDENTIFIER,
				TOKEN_RPAREN,
				TOKEN_LBRACE,
				TOKEN_RETURN,
				TOKEN_IDENTIFIER,
				TOKEN_RBRACE,
				TOKEN_EOF,
			},
			{0},
		},
		{
			"if (condition == 1) { }",
			{
				TOKEN_IF,
				TOKEN_LPAREN,
				TOKEN_IDENTIFIER,
				TOKEN_EQUAL,
				TOKEN_INTEGER,
				TOKEN_RPAREN,
				TOKEN_LBRACE,
				TOKEN_RBRACE,
				TOKEN_EOF,
			},
			{0},
		},
		{
			"let c = 1231232",
			{
				TOKEN_LET,
				TOKEN_IDENTIFIER,
				TOKEN_ASSIGN,
				TOKEN_INTEGER,
				TOKEN_EOF,
			},
			{0},
		},
		{
			"",
			{
				TOKEN_EOF,
			},
			{0},
		},
	};

	struct Lexer lexer;
	int ret = 0;
	unsigned int i = 0;
	unsigned int j;

	while (i < (sizeof(tests) / sizeof(struct TestCase))) {
		lexer_init(&lexer, tests[i].input);
		j = 0;
		do {
			tests[i].got[j] = (lexer_next_token(&lexer)).type;
			if (tests[i].got[j] != tests[i].expected[j])
			{
				print_bad_case(tests[i], j);
				goto next;
			}
		} while(tests[i].expected[j] != TOKEN_EOF && j++);
		next:
		++i;
	}
	if (ret == 0)
		printf("lexer_tests: OK\n");
	return (ret);
}
