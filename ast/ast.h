#ifndef AST_H
# define AST_H

# include "libs/string.h"

enum OpType {
	BINARY_ADD,
	BINARY_MINUS,
	BINARY_MULTIPLY,
	BINARY_DIVIDE,
	BINARY_MODULO,
	UNARY_MINUS,
	BINARY_EQUAL,
	BINARY_NOT_EQUAL,
	BINARY_LT,
	BINARY_GT,
	BINARY_ASSIGN,
};

struct LetStatement {
	struct AstNode *identifier;
	struct AstNode *value;
};

struct ReturnStatement {
	struct AstNode *expr;
};

struct BinaryOp {
	struct AstNode *left;
	enum OpType op;
	struct AstNode *right;
};

struct ArrayAccess {
	struct AstNode *identifier;
	struct AstNode *index;
};

struct FunctionCall {
	struct AstNode *identifier;
	struct AstNode *arguments;
};

struct FunctionDefinition {
	struct AstNode *identifier;
	struct AstNode **parameters;
	struct AstNode *block;
};

struct ListExpression {
	struct AstNode **list;
};

struct IfStatement {
	struct AstNode *cond;
	struct AstNode *block;
	struct AstNode *else_block;
};

struct BlockStatement {
	struct AstNode **statements;
};

struct UnaryOp {
	enum OpType op;
	struct AstNode *value;
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

struct AstNode {
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
	struct AstNode **statements;
};

const char *ast_debug_value(enum NodeType type);
void print_node(struct AstNode *node, int i);

#endif
