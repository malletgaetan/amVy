#ifndef EVALUATOR_H
# define EVALUATOR_H

# include "ast/ast.h"
# include "libs/hashmap.h"

enum ValueType {
	VALUE_INTEGER,
	VALUE_FUNCTION,
	VALUE_NONE,
};

enum BlockValueType {
	BLOCK_RETURN,
	BLOCK_NO_RETURN
};

struct EvalFunction {
	struct AstNode *function_definition;
};

struct EvalValue {
	enum ValueType type;
	union {
		struct EvalFunction eval_function;
		int integer;
	} value;
};

struct BlockEvalValue {
	enum BlockValueType type;
	struct EvalValue val;
};

struct EvalValueMap {
	struct String identifier;
	struct EvalValue val;
};

void evaluator_eval(struct Program program);

#endif
