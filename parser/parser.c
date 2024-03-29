#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "libs/vector.h"
#include "libs/tracer.h"
#include "token/token.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/context.h"

# define PARSER_TRACE "parser"


// TODO: improve performance by using an arena allocator instead of plain malloc
//		 -> following idea would be to make sure parent / child childs are close next to each other
//		 in memory for better cache hit rate.

// TODO: add && and || operator

struct Context *ctx;

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

static struct AstNode *parseExpression(struct Parser *parser, int precedence);
static struct AstNode **parseStatementsUntil(struct Parser *parser, enum TokenType end);

static void expected(struct Parser *parser, enum TokenType expected, enum TokenType got)
{
	if (expected == got)
		return ;
	trace_display();
	printf("%s: expected %s, got %s at line %zu", PARSER_TRACE, token_debug_value(expected), token_debug_value(got), parser->lexer.line);
	exit(1);
}

static void unexpected_error(struct Parser *parser, enum TokenType unexpected)
{
	trace_display();
	printf("%s: unexpected token %s at line %zu", PARSER_TRACE, token_debug_value(unexpected), parser->lexer.line);
	exit(1);
}

static struct AstNode *noneAst(void)
{
	struct AstNode *none = malloc(sizeof(struct AstNode));
	assert(none);
	none->type = AST_NONE;
	return none;
}

static struct AstNode *astBuiltin(enum TokenType type)
{
	struct AstNode *builtin = malloc(sizeof(struct AstNode));
	assert(builtin);
	builtin->type = AST_BUILTIN;
	builtin->node.builtin.type = type;
	return builtin;
}

static void next_token(struct Parser *parser)
{
	parser->token = parser->next_token;
	parser->next_token = lexer_next_token(&parser->lexer);

	if (parser->next_token.type == TOKEN_UNKNOWN)
	{
		printf("%s: failed to recognize token at line %zu", PARSER_TRACE, parser->lexer.line); // TODO: add column
		exit(1);
	}
}

static struct AstNode *parseInteger(struct Parser *parser)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	assert(parser->token.type == TOKEN_INTEGER);
	struct AstNode *node = malloc(sizeof(struct AstNode));
	assert(node);
	node->type = AST_INTEGER_LITERAL;
	node->node.integer_literal.value = atoi(parser->token.literal.str);
	return node;
}

static struct AstNode *parseIdentifier(struct Parser *parser, int create)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	assert(parser->token.type == TOKEN_IDENTIFIER);

	struct AstNode *node = context_get_addr(ctx, parser->token.literal);
	if (node != NULL)
		return node;
	if (!create) {
		printf("parser: accessing undefined variable at line %zu\n", parser->lexer.line); // TODO: better error handling
		trace_display();
		exit(1);
	}
	struct AstNode tmp;
	tmp.type = AST_IDENTIFIER;
	tmp.node.identifier.name = parser->token.literal;
	struct AstNode *a = context_set_addr(ctx, &tmp);
	assert(a);
	return a;
}

static struct AstNode *parseArrayAccessExpression(struct Parser *parser, struct AstNode *left)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	assert(parser->token.type == TOKEN_LBRACKET);
	struct AstNode *expr = malloc(sizeof(struct AstNode));
	assert(expr);

	// TODO: maybe create a special IndexAccess struct for clearer source code
	expr->type = AST_ARRAY_ACCESS;
	expr->node.array_access.identifier = left;
	next_token(parser);
	expr->node.array_access.index = parseExpression(parser, 0);
	next_token(parser);
	expected(parser, TOKEN_RBRACKET, parser->token.type);
	return expr;
}

static struct AstNode *parseListExpression(struct Parser *parser, enum TokenType end)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
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
		expected(parser, TOKEN_COMMA, parser->token.type);
		next_token(parser);
	}

	return list_expr;
}

static struct AstNode *parseFunctionCallExpression(struct Parser *parser, struct AstNode *left)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	struct AstNode *expr = malloc(sizeof(struct AstNode));
	assert(expr);

	// TODO: maybe create a special IndexAccess struct for clearer source code
	assert(parser->token.type == TOKEN_LPAREN);
	expr->type = AST_FUNCTION_CALL;
	expr->node.function_call.identifier = left;
	expr->node.function_call.arguments = parseListExpression(parser, TOKEN_RPAREN);
	expected(parser, TOKEN_RPAREN, parser->token.type);
	return expr;
}

static struct AstNode *parseInfixExpression(struct Parser *parser, struct AstNode *left)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
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
			unexpected_error(parser, parser->token.type);
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
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	assert(parser->token.type == TOKEN_LPAREN);
	struct AstNode *expr;
	next_token(parser);
	// TODO: there's a pattern about this if else, make it innerParseExpression and outerParseExpression, outerParseExpression having a TOKEN_END token
	if (parser->token.type != TOKEN_RPAREN) {
		expr = parseExpression(parser, 0);
		next_token(parser);
	} else {
		expr = noneAst();
	}
	expected(parser, TOKEN_RPAREN, parser->token.type);
	return expr;
}

static struct AstNode *parsePrefixExpression(struct Parser *parser)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	enum OpType op;

	switch (parser->token.type)
	{
		case TOKEN_MINUS:
			op = UNARY_MINUS;
			break;
		case TOKEN_PRINT: // TODO: bad way to handle buitlin.. quick hack for the moment
			return astBuiltin(TOKEN_PRINT);
		case TOKEN_IDENTIFIER:
			return parseIdentifier(parser, 0);
		case TOKEN_INTEGER:
			return parseInteger(parser);
		case TOKEN_LPAREN:
			return parseGroupedExpression(parser);
		case TOKEN_RPAREN:
			return noneAst();
		default:
			unexpected_error(parser, parser->token.type);
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
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	if (parser->token.type == TOKEN_SEMICOLON)
		unexpected_error(parser, TOKEN_SEMICOLON);
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
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	struct AstNode *node = malloc(sizeof(struct AstNode));
	assert(node);
	node->type = AST_LET_STATEMENT;

	next_token(parser);
	node->node.let_statement.identifier = parseIdentifier(parser, 1);
	next_token(parser);

	expected(parser, TOKEN_ASSIGN, parser->token.type);

	next_token(parser);
	if (parser->token.type != TOKEN_SEMICOLON) {
		node->node.let_statement.value = parseExpression(parser, 0);
		next_token(parser);
	} else {
		node->node.let_statement.value = noneAst();
	}
	expected(parser, TOKEN_SEMICOLON, parser->token.type);
	return node;
}

static struct AstNode *parseReturnStatement(struct Parser *parser)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	struct AstNode *node = malloc(sizeof(struct AstNode));
	assert(node);
	node->type = AST_RETURN_STATEMENT;
	next_token(parser);

	if (parser->token.type != TOKEN_SEMICOLON) {
		node->node.return_statement.expr = parseExpression(parser, 0);
		next_token(parser);
	} else {
		node->node.return_statement.expr = noneAst();
	}

	expected(parser, TOKEN_SEMICOLON, parser->token.type);
	return node;
}

static struct AstNode *parseBlockStatement(struct Parser *parser)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	struct AstNode *node = malloc(sizeof(struct AstNode));
	assert(node);

	expected(parser, TOKEN_LBRACE, parser->token.type);
	next_token(parser);

	node->type = AST_BLOCK_STATEMENT;
	node->node.block_statement.statements = parseStatementsUntil(parser, TOKEN_RBRACE);
	expected(parser, TOKEN_RBRACE, parser->token.type);
	return node;
}

static struct AstNode *parseIfStatement(struct Parser *parser)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	assert(parser->token.type == TOKEN_IF);
	struct AstNode *node = malloc(sizeof(struct AstNode));
	node->type = AST_IF_STATEMENT;
	node->node.if_statement.else_block = NULL;
	next_token(parser);

	expected(parser, TOKEN_LPAREN, parser->token.type);
	next_token(parser);
	if (parser->token.type != TOKEN_RPAREN) {
		node->node.if_statement.cond = parseExpression(parser, 0);
		next_token(parser);
	} else {
		node->node.if_statement.cond = noneAst();
	}
	expected(parser, TOKEN_RPAREN, parser->token.type);
	next_token(parser);

	expected(parser, TOKEN_LBRACE, parser->token.type);
	node->node.if_statement.block = parseBlockStatement(parser);
	expected(parser, TOKEN_RBRACE, parser->token.type);
	if (parser->next_token.type != TOKEN_ELSE)
		return node;

	next_token(parser);
	next_token(parser);
	expected(parser, TOKEN_LBRACE, parser->token.type);
	node->node.if_statement.else_block = parseBlockStatement(parser);
	expected(parser, TOKEN_RBRACE, parser->token.type);
	return node;
}

static struct AstNode *parseFunctionDefinition(struct Parser *parser)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	assert(parser->token.type == TOKEN_FUNCTION);
	// TODO: each new node allocation should init all fields (NULL init)
	struct AstNode *node = malloc(sizeof(struct AstNode));
	node->type = AST_FUNCTION_DEFINITION;
	node->node.function_definition.parameters = NULL;
	node->node.function_definition.block = NULL;

	next_token(parser);
	expected(parser, TOKEN_IDENTIFIER, parser->token.type);
	node->node.function_definition.identifier = parseIdentifier(parser, 1); // create function outside function scope

	struct Context new_context;
	struct Context *old_context = ctx;
	context_init(&new_context);
	ctx = &new_context;

	node->node.function_definition.identifier = parseIdentifier(parser, 1); // create function inside function scope
	next_token(parser);
	expected(parser, TOKEN_LPAREN, parser->token.type);

	next_token(parser);
	while (parser->token.type != TOKEN_RPAREN)
	{
		if (parser->token.type == TOKEN_COMMA)
			next_token(parser);
		struct AstNode *identifier = parseIdentifier(parser, 1);
		vector_add(node->node.function_definition.parameters, identifier);
		next_token(parser);
	}
	next_token(parser);
	node->node.function_definition.block = parseBlockStatement(parser);
	ctx = old_context; // TODO: here we loose pointer to ctx->hashmap, we don't really care but it still have to be noticed
	return (node);
}

static struct AstNode *parseWhileStatement(struct Parser *parser)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	assert(parser->token.type == TOKEN_WHILE);
	struct AstNode *node = malloc(sizeof(struct AstNode));
	node->type = AST_WHILE_STATEMENT;
	next_token(parser);

	expected(parser, TOKEN_LPAREN, parser->token.type);
	next_token(parser);
	if (parser->token.type != TOKEN_RPAREN) {
		node->node.while_statement.cond = parseExpression(parser, 0);
		next_token(parser);
	} else {
		node->node.while_statement.cond = noneAst();
	}
	expected(parser, TOKEN_RPAREN, parser->token.type);
	next_token(parser);

	expected(parser, TOKEN_LBRACE, parser->token.type);
	node->node.while_statement.block = parseBlockStatement(parser);
	expected(parser, TOKEN_RBRACE, parser->token.type);
	return node;
}

static struct AstNode *parseStatement(struct Parser *parser)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
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
		case TOKEN_WHILE:
			return parseWhileStatement(parser);
		case TOKEN_FUNCTION:
			return parseFunctionDefinition(parser);
		case TOKEN_SEMICOLON:
			return NULL;
		default:
			statement = parseExpression(parser, 0);
			next_token(parser);
			break;
	}
	expected(parser, TOKEN_SEMICOLON, parser->token.type);
	return statement;
}

static struct AstNode **parseStatementsUntil(struct Parser *parser, enum TokenType end)
{
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
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
	trace("%s: %s(%s)", PARSER_TRACE, __func__, token_debug_value(parser->token.type));
	struct Program program;
	struct Context context;
	context_init(&context);
	ctx = &context;
	program.statements = parseStatementsUntil(parser, TOKEN_EOF);
	lexer_zero_string(&parser->lexer);
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

