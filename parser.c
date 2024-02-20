#include "parser.h"
#include "vector.h"
#include "types.h"


// TODO: improve performance by using an arena allocator instead of plain malloc
//		 -> following idea would be to make sure parent / child childs are close next to each other
//		 in memory for better cache hit rate.

static void next_token(struct Parser *parser)
{
	parser->token = parser->next_token;
	parser->next_token = lexer_next_token(&parser->lexer);
}

bool parser_init(struct Parser *parser, char *filepath)
{
	return lexer_init(&parser->lexer, filepath);
}

void parser_destroy(struct Parser *parser)
{
	return lexer_destroy(&parser->lexer);
}

struct Node *parseLetStatement(struct Parser *parser)
{
}

struct Program parser_parse(struct Parser *parser)
{
	struct Program program;

	program.statements = NULL;

	next_token(parser);
	while (program.token.type != TOKEN_EOF)
	{
		switch (program.token.type)
		{
			case TOKEN_LET:
				vector_add(program.statements, parseLetStatement(parser));
			default:
				printf("BAD PROGRAM\n");
				return program;
		}
	}

	return program;
}
