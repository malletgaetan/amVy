#ifndef AST_H
# define AST_H

# include "string.h"

enum OpType {
	BINARY_ADD,
	BINARY_MINUS,
	BINARY_MULTIPLY,
	BINARY_DIVIDE,
	UNARY_MINUS,
	BINARY_ARRAY_ACCESS,
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
	} node;
};

struct Program {
	struct Node **statements;
};

const char *ast_debug_value(enum NodeType type);
void print_node(struct Node *node, int i);

#endif
