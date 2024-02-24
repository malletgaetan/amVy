#include <stdio.h>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "vector.h"
#include "assert.h"
#include "types.h"


// TODO: improve performance by using an arena allocator instead of plain malloc
//		 -> following idea would be to make sure parent / child childs are close next to each other
//		 in memory for better cache hit rate.

// TODO: add && and || operator

static int	token_precedence[TOKEN_LIMIT] =
{
	[TOKEN_PLUS] = 10,
	[TOKEN_MINUS] = 10,
	[TOKEN_ASTERISK] = 20,
	[TOKEN_SLASH] = 20,
	[TOKEN_EQUAL] = 5,
	[TOKEN_NOT_EQUAL] = 5,
	[TOKEN_LESSER] = 5,
	[TOKEN_GREATER] = 5,
	[TOKEN_LPAREN] = 50,
	[TOKEN_LBRACKET] = 60,
};

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

static struct Node *parseExpression(struct Parser *parser, int precedence);
static struct Node **parseStatementsUntil(struct Parser *parser, enum TokenType end);

static void next_token(struct Parser *parser)
{

	parser->token = parser->next_token;
	parser->next_token = lexer_next_token(&parser->lexer);
	assert(parser->next_token.type != TOKEN_UNKNOWN);
}

static struct Node *parseInteger(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_INTEGER);
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);
	node->type = AST_INTEGER_LITERAL;
	node->node.integer_literal.value = nzatoi(parser->token.literal);
	return node;
}

static struct Node *parseIdentifier(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_IDENTIFIER);
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);
	node->type = AST_IDENTIFIER;
	node->node.identifier.name = parser->token.literal;
	return node;
}

static struct Node *parseArrayAccessExpression(struct Parser *parser, struct Node *left)
{
	assert(parser->token.type == TOKEN_LBRACKET);
	struct Node *expr = malloc(sizeof(struct ArrayAccess));
	assert(expr);

	// TODO: maybe create a special IndexAccess struct for clearer source code
	expr->type = AST_ARRAY_ACCESS;
	expr->node.array_access.identifier = left;
	next_token(parser);
	expr->node.array_access.index = parseExpression(parser, 0);
	next_token(parser);
	assert(parser->token.type == TOKEN_RBRACKET);
	return expr;
}

static struct Node *parseListExpression(struct Parser *parser, enum TokenType end)
{
	struct Node *list_expr = malloc(sizeof(struct ListExpression));
	assert(list_expr);

	list_expr->type = AST_LIST_EXPRESSION;
	list_expr->node.list_expression.list = NULL;
	next_token(parser);
	if (parser->token.type == end)
		return list_expr;
	
	while (1)
	{
		struct Node *expr = parseExpression(parser, 0);
		assert(expr);
		vector_add(list_expr->node.list_expression.list, expr);
		next_token(parser);
		if (parser->token.type == end)
			break;
		assert(parser->token.type == TOKEN_COMMA);
		next_token(parser);
	}

	return list_expr;
}

static struct Node *parseFunctionCallExpression(struct Parser *parser, struct Node *left)
{
	struct Node *expr = malloc(sizeof(struct FunctionCall));
	assert(expr);

	// TODO: maybe create a special IndexAccess struct for clearer source code
	assert(parser->token.type == TOKEN_LPAREN);
	expr->type = AST_FUNCTION_CALL;
	expr->node.function_call.identifier = left;
	expr->node.function_call.arguments = parseListExpression(parser, TOKEN_RPAREN);
	assert(parser->token.type == TOKEN_RPAREN);
	return expr;
}

static struct Node *parseInfixExpression(struct Parser *parser, struct Node *left)
{
	enum OpType op;
	switch (parser->token.type)
	{
		case TOKEN_PLUS:
			op = BINARY_ADD;
			break;
		case TOKEN_MINUS:
			op = BINARY_MINUS;
			break;
		case TOKEN_ASTERISK:
			op = BINARY_MULTIPLY;
			break;
		case TOKEN_SLASH:
			op = BINARY_DIVIDE;
			break;
		case TOKEN_LBRACKET:
			return parseArrayAccessExpression(parser, left);
		case TOKEN_LPAREN:
			return parseFunctionCallExpression(parser, left);
		case TOKEN_EQUAL:
			op = BINARY_EQUAL;
			break;
		case TOKEN_NOT_EQUAL:
			op = BINARY_NOT_EQUAL;
			break;
		case TOKEN_LESSER:
			op = BINARY_LT;
			break;
		case TOKEN_GREATER:
			op = BINARY_GT;
			break;
		case TOKEN_RPAREN:
		case TOKEN_RBRACKET:
			return left;
		default:
			printf("%s\n", token_debug_value(parser->token.type));
			assert(NULL);
	}
	struct Node *expr = malloc(sizeof(struct Node));
	assert(expr);
	expr->type = AST_BINARY_OP;
	expr->node.binary_op.left = left;
	expr->node.binary_op.op = op;
	int precedence = token_precedence[parser->token.type];
	next_token(parser);
	expr->node.binary_op.right = parseExpression(parser, precedence);
	return expr;
}

static struct Node *parseGroupedExpression(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_LPAREN);
	next_token(parser);
	struct Node *expr = parseExpression(parser, 0);
	next_token(parser);
	assert(parser->token.type == TOKEN_RPAREN);
	return expr;
}

static struct Node *parsePrefixExpression(struct Parser *parser)
{
	enum OpType op;

	switch (parser->token.type)
	{
		case TOKEN_MINUS:
			op = UNARY_MINUS;
			break;
		case TOKEN_IDENTIFIER:
			return parseIdentifier(parser);
		case TOKEN_INTEGER:
			return parseInteger(parser);
		case TOKEN_LPAREN:
			return parseGroupedExpression(parser);
		default:
			printf("%s\n", token_debug_value(parser->token.type));
			assert(NULL);
	}
	struct Node *expr = malloc(sizeof(struct Node));
	assert(expr);
	expr->type = AST_UNARY_OP;
	int precedence = token_precedence[parser->token.type];
	next_token(parser);
	expr->node.unary_op.value = parseExpression(parser, precedence);
	expr->node.unary_op.op = op;
	return expr;
}

static struct Node *parseExpression(struct Parser *parser, int precedence)
{
	struct Node *left = parsePrefixExpression(parser);

	// && printf("test on %s\n", token_debug_value(parser->next_token.type))
	while (parser->token.type != TOKEN_EOF && precedence < token_precedence[parser->next_token.type])
	{
		next_token(parser);
		left = parseInfixExpression(parser, left);
	}
	return left;
}

static struct Node *parseLetStatement(struct Parser *parser)
{
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);
	node->type = AST_LET_STATEMENT;

	next_token(parser);
	node->node.let_statement.identifier = parseIdentifier(parser);
	next_token(parser);

	assert(parser->token.type == TOKEN_ASSIGN);

	next_token(parser);
	node->node.let_statement.value = parseExpression(parser, 0);
	next_token(parser);
	assert(parser->token.type == TOKEN_SEMICOLON);
	return node;
}

static struct Node *parseReturnStatement(struct Parser *parser)
{
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);
	node->type = AST_RETURN_STATEMENT;
	next_token(parser);
	node->node.return_statement.expr = parseExpression(parser, 0);
	next_token(parser);
	assert(parser->token.type == TOKEN_SEMICOLON);
	return node;
}

static struct Node *parseBlockStatement(struct Parser *parser)
{
	struct Node *node = malloc(sizeof(struct Node));
	assert(node);

	assert(parser->token.type == TOKEN_LBRACE);
	next_token(parser);

	node->type = AST_BLOCK_STATEMENT;
	node->node.block_statement.statements = parseStatementsUntil(parser, TOKEN_RBRACE);
	assert(parser->token.type == TOKEN_RBRACE);
	return node;
}

static struct Node *parseIfStatement(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_IF);
	struct Node *node = malloc(sizeof(struct Node));
	node->type = AST_IF_STATEMENT;
	node->node.if_statement.else_block = NULL;
	next_token(parser);

	assert(parser->token.type == TOKEN_LPAREN);
	next_token(parser);
	node->node.if_statement.cond = parseExpression(parser, 0);
	next_token(parser);
	assert(parser->token.type == TOKEN_RPAREN);
	next_token(parser);

	assert(parser->token.type == TOKEN_LBRACE);
	node->node.if_statement.block = parseBlockStatement(parser);
	assert(parser->token.type == TOKEN_RBRACE);
	if (parser->next_token.type != TOKEN_ELSE)
		return node;

	next_token(parser);
	next_token(parser);
	assert(parser->token.type == TOKEN_LBRACE);
	node->node.if_statement.else_block = parseBlockStatement(parser);
	assert(parser->token.type == TOKEN_RBRACE);
	return node;
}

static struct Node *parseStatement(struct Parser *parser)
{
	struct Node *statement;

	switch (parser->token.type)
	{
		case TOKEN_LET:
			statement = parseLetStatement(parser);
			break;
		case TOKEN_RETURN:
			statement = parseReturnStatement(parser);
			break;
		case TOKEN_IF:
			return parseIfStatement(parser);
		default:
			printf("%s\n", token_debug_value(parser->token.type));
			assert(NULL);
	}
	assert(parser->token.type == TOKEN_SEMICOLON);
	return statement;
}

static struct Node **parseStatementsUntil(struct Parser *parser, enum TokenType end)
{
	struct Node *statement;
	struct Node **statements = NULL;

	do {
		statement = parseStatement(parser);
		if (statement != NULL)
			vector_add(statements, statement);
		next_token(parser);
	} while (parser->token.type != end);

	return statements;
}

struct Program parser_parse(struct Parser *parser)
{
	struct Program program;

	program.statements = parseStatementsUntil(parser, TOKEN_EOF);
	return program;
}

bool parser_init(struct Parser *parser, char *filepath)
{
	bool ret = lexer_init(&parser->lexer, filepath);
	next_token(parser);
	next_token(parser);
	return (ret);
}

void parser_destroy(struct Parser *parser)
{
	return lexer_destroy(&parser->lexer);
}

