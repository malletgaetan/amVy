#include <assert.h>
#include <stdio.h>

#include "libs/vector.h"
#include "libs/types.h"
#include "token/token.h"
#include "lexer/lexer.h"
#include "parser/parser.h"


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
	[TOKEN_MODULO] = 20,
	[TOKEN_ASSIGN] = 4,
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

static struct AstNode *parseExpression(struct Parser *parser, int precedence);
static struct AstNode **parseStatementsUntil(struct Parser *parser, enum TokenType end);

static void next_token(struct Parser *parser)
{

	parser->token = parser->next_token;
	parser->next_token = lexer_next_token(&parser->lexer);
	assert(parser->next_token.type != TOKEN_UNKNOWN);
}

static struct AstNode *parseInteger(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_INTEGER);
	struct AstNode *node = malloc(sizeof(struct AstNode));
	assert(node);
	node->type = AST_INTEGER_LITERAL;
	node->node.integer_literal.value = nzatoi(parser->token.literal);
	return node;
}

static struct AstNode *parseIdentifier(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_IDENTIFIER);
	struct AstNode *node = malloc(sizeof(struct AstNode));
	assert(node);
	node->type = AST_IDENTIFIER;
	node->node.identifier.name = parser->token.literal;
	return node;
}

static struct AstNode *parseArrayAccessExpression(struct Parser *parser, struct AstNode *left)
{
	assert(parser->token.type == TOKEN_LBRACKET);
	struct AstNode *expr = malloc(sizeof(struct AstNode));
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

static struct AstNode *parseListExpression(struct Parser *parser, enum TokenType end)
{
	struct AstNode *list_expr = malloc(sizeof(struct AstNode));
	assert(list_expr);

	list_expr->type = AST_LIST_EXPRESSION;
	list_expr->node.list_expression.list = NULL;
	next_token(parser);
	if (parser->token.type == end)
		return list_expr;
	
	while (1)
	{
		struct AstNode *expr = parseExpression(parser, 0);
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

static struct AstNode *parseFunctionCallExpression(struct Parser *parser, struct AstNode *left)
{
	struct AstNode *expr = malloc(sizeof(struct AstNode));
	assert(expr);

	// TODO: maybe create a special IndexAccess struct for clearer source code
	assert(parser->token.type == TOKEN_LPAREN);
	expr->type = AST_FUNCTION_CALL;
	expr->node.function_call.identifier = left;
	expr->node.function_call.arguments = parseListExpression(parser, TOKEN_RPAREN);
	assert(parser->token.type == TOKEN_RPAREN);
	return expr;
}

static struct AstNode *parseInfixExpression(struct Parser *parser, struct AstNode *left)
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
		case TOKEN_MODULO:
			op = BINARY_MODULO;
			break;
		case TOKEN_ASSIGN:
			op = BINARY_ASSIGN;
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
	struct AstNode *expr = malloc(sizeof(struct AstNode));
	assert(expr);
	expr->type = AST_BINARY_OP;
	expr->node.binary_op.left = left;
	expr->node.binary_op.op = op;
	int precedence = token_precedence[parser->token.type];
	next_token(parser);
	expr->node.binary_op.right = parseExpression(parser, precedence);
	return expr;
}

static struct AstNode *parseGroupedExpression(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_LPAREN);
	next_token(parser);
	struct AstNode *expr = parseExpression(parser, 0);
	next_token(parser);
	assert(parser->token.type == TOKEN_RPAREN);
	return expr;
}

static struct AstNode *parsePrefixExpression(struct Parser *parser)
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
	struct AstNode *expr = malloc(sizeof(struct AstNode));
	assert(expr);
	expr->type = AST_UNARY_OP;
	int precedence = token_precedence[parser->token.type];
	next_token(parser);
	expr->node.unary_op.value = parseExpression(parser, precedence);
	expr->node.unary_op.op = op;
	return expr;
}

static struct AstNode *parseExpression(struct Parser *parser, int precedence)
{
	struct AstNode *left = parsePrefixExpression(parser);

	// && printf("test on %s\n", token_debug_value(parser->next_token.type))
	while (parser->token.type != TOKEN_SEMICOLON && precedence < token_precedence[parser->next_token.type])
	{
		next_token(parser);
		left = parseInfixExpression(parser, left);
	}
	return left;
}

static struct AstNode *parseLetStatement(struct Parser *parser)
{
	struct AstNode *node = malloc(sizeof(struct AstNode));
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

static struct AstNode *parseReturnStatement(struct Parser *parser)
{
	struct AstNode *node = malloc(sizeof(struct AstNode));
	assert(node);
	node->type = AST_RETURN_STATEMENT;
	next_token(parser);
	node->node.return_statement.expr = parseExpression(parser, 0);
	next_token(parser);
	assert(parser->token.type == TOKEN_SEMICOLON);
	return node;
}

static struct AstNode *parseBlockStatement(struct Parser *parser)
{
	struct AstNode *node = malloc(sizeof(struct AstNode));
	assert(node);

	assert(parser->token.type == TOKEN_LBRACE);
	next_token(parser);

	node->type = AST_BLOCK_STATEMENT;
	node->node.block_statement.statements = parseStatementsUntil(parser, TOKEN_RBRACE);
	assert(parser->token.type == TOKEN_RBRACE);
	return node;
}

static struct AstNode *parseIfStatement(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_IF);
	struct AstNode *node = malloc(sizeof(struct AstNode));
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

static struct AstNode *parseFunctionDefinition(struct Parser *parser)
{
	assert(parser->token.type == TOKEN_FUNCTION);
	// TODO: each new node allocation should init all fields (NULL init)
	struct AstNode *node = malloc(sizeof(struct AstNode));
	node->type = AST_FUNCTION_DEFINITION;
	node->node.function_definition.parameters = NULL;
	node->node.function_definition.block = NULL;

	next_token(parser);
	node->node.function_definition.identifier = parseIdentifier(parser);
	next_token(parser);
	assert(parser->token.type == TOKEN_LPAREN);

	next_token(parser);
	while (parser->token.type != TOKEN_RPAREN)
	{
		if (parser->token.type == TOKEN_COMMA)
			next_token(parser);
		struct AstNode *identifier = parseIdentifier(parser);
		vector_add(node->node.function_definition.parameters, identifier);
		next_token(parser);
	}
	next_token(parser);
	node->node.function_definition.block = parseBlockStatement(parser);
	return (node);
}

static struct AstNode *parseStatement(struct Parser *parser)
{
	struct AstNode *statement;

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
		case TOKEN_FUNCTION:
			return parseFunctionDefinition(parser);
		default:
			statement = parseExpression(parser, 0);
			next_token(parser);
			break;
	}
	assert(parser->token.type == TOKEN_SEMICOLON);
	return statement;
}

static struct AstNode **parseStatementsUntil(struct Parser *parser, enum TokenType end)
{
	struct AstNode *statement;
	struct AstNode **statements = NULL;

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

void parser_init(struct Parser *parser, char *file_content)
{
	lexer_init(&parser->lexer, file_content);
	next_token(parser);
	next_token(parser);
}

void parser_destroy(struct Parser *parser)
{
	return lexer_destroy(&parser->lexer);
}

