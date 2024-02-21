#ifndef AST_H
# define AST_H

# include "string.h"

enum BinaryOpType {
	BINARY_ADD,
	BINARY_MINUS,
	BINARY_MULTIPLY,
	BINARY_DIVIDE,
};

enum UnaryOpType {
	UNARY_MINUS,
	UNARY_INVERSE,
};

struct LetStatement {
	struct Node *identifier;
	struct Node *value;
};

struct BinaryOp {
	struct Node *left;
	enum BinaryOpType op;
	struct Node *right;
};

struct UnaryOp {
	enum UnaryOpType op;
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
};

struct Node {
	enum NodeType type;
	union {
		struct LetStatement let_statement;
		struct BinaryOp binary_op;
		struct UnaryOp unary_op;
		struct Identifier identifier;
		struct IntegerLiteral integer_literal;
	} node;
};

struct Program {
	struct Node **statements;
};

const char *ast_debug_value(enum NodeType type);
void print_node(struct Node *node, int i);

#endif
