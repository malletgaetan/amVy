#include <stdio.h>
#include "parser.h"
#include "ast.h"
#include "vector.h"

int main(int ac, char **av)
{
	if (ac < 2)
	{
		printf("nothing to do\n");
		return (0);
	}

	struct Parser parser;

	parser_init(&parser, av[1]);
	struct Program program = parser_parse(&parser);
	for (unsigned int i = 0; i < vector_size(program.statements); i++)
	{
		print_node(program.statements[i], 0);
	}
	parser_destroy(&parser);
	return (0);
}

