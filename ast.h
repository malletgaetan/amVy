#ifndef AST_H
# define AST_H

# include "string.h"

enum OpType {
	BINARY_ADD,
	BINARY_MINUS,
	BINARY_MULTIPLY,
	BINARY_DIVIDE,
	UNARY_MINUS,
	BINARY_EQUAL,
	BINARY_NOT_EQUAL,
	BINARY_LT,
	BINARY_GT,
	BINARY_ASSIGN,
};

struct LetStatement {
	struct Node *identifier;
	struct Node *value;
};

struct ReturnStatement {
	struct Node *expr;
};

struct BinaryOp {
	struct Node *left;
	enum OpType op;
	struct Node *right;
};

struct ArrayAccess {
	struct Node *identifier;
	struct Node *index;
};

struct FunctionCall {
	struct Node *identifier;
	struct Node *arguments;
};

struct FunctionDefinition {
	struct Node *identifier;
	struct Node **parameters;
	struct Node *block;
};

struct ListExpression {
	struct Node **list;
};

struct IfStatement {
	struct Node *cond;
	struct Node *block;
	struct Node *else_block;
};

struct BlockStatement {
	struct Node **statements;
};

struct UnaryOp {
	enum OpType op;
	struct Node *value;
};

struct Identifier {
	struct String name;
};

struct IntegerLiteral {
	int value;
};

enum NodeType {
	AST_LET_STATEMENT,
	AST_IDENTIFIER,
	AST_BINARY_OP,
	AST_UNARY_OP,
	AST_INTEGER_LITERAL,
	AST_RETURN_STATEMENT,
	AST_ARRAY_ACCESS,
	AST_FUNCTION_CALL,
	AST_LIST_EXPRESSION,
	AST_IF_STATEMENT,
	AST_BLOCK_STATEMENT,
	AST_FUNCTION_DEFINITION,
};

struct Node {
	enum NodeType type;
	union {
		struct LetStatement let_statement;
		struct BinaryOp binary_op;
		struct UnaryOp unary_op;
		struct Identifier identifier;
		struct IntegerLiteral integer_literal;
		struct ReturnStatement return_statement;
		struct ArrayAccess array_access;
		struct FunctionCall function_call;
		struct ListExpression list_expression;
		struct IfStatement if_statement;
		struct BlockStatement block_statement;
		struct FunctionDefinition function_definition;
	} node;
};

struct Program {
	struct Node **statements;
};

const char *ast_debug_value(enum NodeType type);
void print_node(struct Node *node, int i);

#endif
