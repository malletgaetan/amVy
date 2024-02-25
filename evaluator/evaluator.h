#ifndef EVALUATOR_H
# define EVALUATOR_H

enum ValueType {
	VALUE_INTEGER,
	VALUE_FUNCTION,
};

struct Context {
	struct FunctionValue **functions;
};

struct IntegerValue {
	int value;
};

struct FunctionValue {
	struct Node *function_definition;
	const struct Context *context;
};

struct Value {
	enum ValueType type;
	union {
	} value;
};

#endif
