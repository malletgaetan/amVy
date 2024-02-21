#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "vector.h"
#include "assert.h"
#include "types.h"


// TODO: improve performance by using an arena allocator instead of plain malloc
//		 -> following idea would be to make sure parent / child childs are close next to each other
//		 in memory for better cache hit rate.

int	nzatoi(struct String string)
{
	int	sign = 1;
	int	count = 0;
	unsigned int i = 0;

	while (i < string.size)
	{
		count = (count * 10) + (string.str[i] - '0');
		++i;
	}
	return (count * sign);
}

static struct Node *parseExpression(struct Parser *parser);

static void next_token(struct Parser *parser)
{

	parser->token = parser->next_token;
	parser->next_token = lexer_next_token(&parser->lexer);
	/* printf("next_token=%s\n", token_debug_value(parser->next_token.type)); */
}

static struct Node *parseInteger(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_INTEGER);
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);
	node->type = AST_INTEGER_LITERAL;
	node->node.integer_literal.value = nzatoi(parser->token.literal);
	next_token(parser);
	return node;
}

static struct Node *parseIdentifier(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_IDENTIFIER);
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);
	node->type = AST_IDENTIFIER;
	node->node.identifier.name = parser->token.literal;
	next_token(parser);
	return node;
}

static struct Node *parseValue(struct Parser *parser)
{
	if (parser->token.type == TOKEN_INTEGER)
		return parseInteger(parser);
	if (parser->token.type == TOKEN_IDENTIFIER)
		return parseIdentifier(parser);
	assert(NULL);
}

static struct Node *parseBinaryOp(struct Parser *parser)
{
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);

	node->type = AST_BINARY_OP;
	node->node.binary_op.left = parseValue(parser);
	switch (parser->token.type)
	{
		case TOKEN_PLUS:
			node->node.binary_op.op = BINARY_ADD;
			break;
		case TOKEN_MINUS:
			node->node.binary_op.op = BINARY_MINUS;
			break;
		case TOKEN_ASTERISK:
			node->node.binary_op.op = BINARY_MULTIPLY;
			break;
		case TOKEN_SLASH:
			node->node.binary_op.op = BINARY_DIVIDE;
			break;
		default:
			assert(NULL);
	}
	node->node.binary_op.right = parseExpression(parser);
	return (node);
}

static struct Node *parseLetStatement(struct Parser *parser)
{
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);
	node->type = AST_LET_STATEMENT;

	next_token(parser);
	node->node.let_statement.identifier = parseIdentifier(parser);

	assert(parser->token.type == TOKEN_ASSIGN);
	node->node.let_statement.value = parseExpression(parser);
	assert(parser->token.type == TOKEN_SEMICOLON);
	return node;
}

static struct Node *parseExpression(struct Parser *parser)
{
	next_token(parser);

	switch (parser->token.type)
	{
		case TOKEN_INTEGER:
		case TOKEN_IDENTIFIER:
			switch (parser->next_token.type)
			{
				case TOKEN_PLUS:
				case TOKEN_MINUS:
				case TOKEN_ASTERISK:
				case TOKEN_SLASH:
					return parseBinaryOp(parser);
				case TOKEN_SEMICOLON:
					return parseValue(parser);
				default:
					assert(NULL);
			}
		case TOKEN_PLUS:
		case TOKEN_MINUS:
		case TOKEN_ASTERISK:
		case TOKEN_SLASH:
			/* return parseUnaryOp(parser); */
		default:
			assert(NULL);
	}
}

static struct Node *parseStatement(struct Parser *parser)
{
	switch (parser->token.type)
	{
		case TOKEN_LET:
			return parseLetStatement(parser);
		default:
			assert(NULL);
	}
}

struct Program parser_parse(struct Parser *parser)
{
	struct Program program;
	struct Node *statement;

	program.statements = NULL;

	next_token(parser);
	next_token(parser);
	do {
		statement = parseStatement(parser);
		if (statement != NULL)
			vector_add(program.statements, statement);
		next_token(parser);
	} while (parser->token.type != TOKEN_EOF);

	return program;
}

bool parser_init(struct Parser *parser, char *filepath)
{
	return lexer_init(&parser->lexer, filepath);
}

void parser_destroy(struct Parser *parser)
{
	return lexer_destroy(&parser->lexer);
}

