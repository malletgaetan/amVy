#include <stdio.h>

#include "lexer.h"
#include "token.h"

int main(int ac, char **av)
{
	if (ac < 2)
	{
		printf("TODO\n");
		return (0);
	}
	struct Lexer lexer;

	if (lexer_init(&lexer, av[1]))
	{
		printf("that didn't worked out!\n");
		return (1);
	}

	struct Token token = lexer_next_token(&lexer);
	while (token.type != TOKEN_EOF)
	{
		printf("Got a token of type %s\n", token_debug_value(token.type));
		token = lexer_next_token(&lexer);
	}
	printf("DONE!\n");
	lexer_destroy(&lexer);
}

