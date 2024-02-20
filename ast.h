#ifndef AST_H

# include "string.h"

enum NodeType {
	LET_STATEMENT,
	IDENTIFIER,
	EXPRESSION,
};

struct LetStatement {
	struct Identifier *id;
	struct Expression *value;
};

struct Identifier {
	struct String str;
};

struct Expression {
};

struct Node {
	enum NodeType type;
	union {
		struct LetStatement LET_STATEMENT;
		struct Identifier IDENTIFIER;
		struct Expression EXPRESSION;
	} node;
};

struct Program {
	struct Statement []statements;
};


#endif
