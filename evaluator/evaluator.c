#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <libs/vector.h>
#include "libs/tracer.h"
#include "libs/string.h"
#include "evaluator/evaluator.h"
#include "evaluator/stack.h"

# define EVAL_TRACER "eval"

char *value_debug[] = {
	[VALUE_INTEGER] = "INTEGER",
	[VALUE_FUNCTION] = "FUNCTION",
	[VALUE_NONE] = "NONE",
};

struct Stack stack;


static struct BlockEvalValue evalBlockStatement(struct AstNode *node, size_t offset);
static struct EvalValue evalExpression(struct AstNode *node, size_t offset);

static void runtime_error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	size_t needed = vsnprintf((char*)NULL, 0, fmt, args) + 1;
	va_end(args);
    char  *buffer = malloc(needed);
	assert(needed);
	va_start(args, fmt);
    vsprintf(buffer, fmt, args);
	va_end(args);
	buffer[needed - 1] = '\0';
	printf("runtime_error: %s\n", buffer);
	trace_display();
	exit(1);
}

static void binaryOpTypeCheck(struct EvalValue left, struct EvalValue right)
{
	trace("%s: %s", EVAL_TRACER, __func__);
	if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER)
		return ;
	runtime_error("invalid arithmetic operation between '%s' and '%s' values", value_debug[left.type], value_debug[left.type]);
}

static void expectIdentifier(struct AstNode *node)
{
	trace("%s: %s(%s)", EVAL_TRACER, __func__, ast_debug_value(node->type));
	assert(node);
	if (node->type == AST_IDENTIFIER)
		return ;
	runtime_error("expected identifier, got '%s'", ast_debug_value(node->type));
	exit(1);
}

static int add(int left, int right) { return left + right; }
static int minus(int left, int right) { return left - right; }
static int multiply(int left, int right) { return left * right; }
static int divide(int left, int right) { return left / right; }
static int modulo(int left, int right) { return left % right; }
static int equal(int left, int right) { return left == right; }
static int not_equal(int left, int right) { return left != right; }
static int less_than(int left, int right) { return left < right; }
static int greater_than(int left, int right) { return left > right; }

int(*eval_binary_op[OP_LIMIT])(int, int) = 
{
	[BINARY_ADD] = &add,
	[BINARY_MINUS] = &minus,
	[BINARY_MULTIPLY] = &multiply,
	[BINARY_DIVIDE] = &divide,
	[BINARY_MODULO] = &modulo,
	[BINARY_EQUAL] = &equal,
	[BINARY_NOT_EQUAL] = &not_equal,
	[BINARY_LT] = &less_than,
	[BINARY_GT] = &greater_than,
};

static struct EvalValue evalBinaryOp(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);

	if (node->node.binary_op.op == BINARY_ASSIGN)
	{
		expectIdentifier(node->node.binary_op.left);
		struct EvalValue *var = stack.values + offset + node->node.binary_op.left->node.identifier.offset;
		*var = evalExpression(node->node.binary_op.right, offset);
		return *var;
	}

	struct EvalValue left = evalExpression(node->node.binary_op.left, offset);
	struct EvalValue right = evalExpression(node->node.binary_op.right, offset);

	binaryOpTypeCheck(left, right);
	int(*f)(int, int) = eval_binary_op[node->node.binary_op.op];
	if (f == NULL)
	{
		trace_display();
		assert(NULL);
	}
	left.value.integer = f(left.value.integer, right.value.integer);
	return left;
}

static struct EvalValue evalIdentifier(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);
	assert(node->type == AST_IDENTIFIER);
	struct EvalValue ret = *(stack.values + offset + node->node.identifier.offset);
	return ret;
}

static struct EvalValue evalUnaryOp(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);

	struct EvalValue val = evalExpression(node->node.unary_op.value, offset);

	if (val.type != VALUE_INTEGER)
		runtime_error("unary operator expected '%s', got %s", value_debug[VALUE_INTEGER], value_debug[val.type]);
	val.value.integer = -val.value.integer;
	return val;
}

static struct EvalValue evalIntegerLiteral(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s)", EVAL_TRACER, __func__, ast_debug_value(node->type));
	assert(node);
	(void)offset;

	struct EvalValue val;

	val.type = VALUE_INTEGER;
	val.value.integer = node->node.integer_literal.value;
	return val;
}

static struct BlockEvalValue evalIfStatement(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);
	assert(node->type == AST_IF_STATEMENT);

	struct EvalValue cond = evalExpression(node->node.if_statement.cond, offset);
	struct BlockEvalValue ret;
	ret.type = BLOCK_NO_RETURN;

	if (cond.type != VALUE_INTEGER)
		runtime_error("condition expression expected '%s', got '%s'", value_debug[VALUE_INTEGER], value_debug[cond.type]);
	if (!cond.value.integer && node->node.if_statement.else_block)
		return evalBlockStatement(node->node.if_statement.else_block, offset);
	if (cond.value.integer)
		return evalBlockStatement(node->node.if_statement.block, offset);
	return ret;
}

static struct BlockEvalValue evalWhileStatement(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);
	assert(node->type == AST_WHILE_STATEMENT);

	struct BlockEvalValue ret;
	struct EvalValue cond;
	ret.type = BLOCK_NO_RETURN;
	
	while (1) {
		cond = evalExpression(node->node.if_statement.cond, offset);
		if (cond.type != VALUE_INTEGER)
			runtime_error("condition expression expected '%s', got '%s'", value_debug[VALUE_INTEGER], value_debug[cond.type]);
		if (!cond.value.integer)
			break ; // stop execution because condition not satisfied
		ret = evalBlockStatement(node->node.while_statement.block, offset);
		if (ret.type == BLOCK_RETURN)
			break ; // stop execution because return statement inside while
	}
	return ret;
}

static struct EvalValue evalFunctionDefinition(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);
	assert(node->type == AST_FUNCTION_DEFINITION);

	(void)offset;
	struct EvalValue fn;

	fn.type = VALUE_FUNCTION;
	fn.value.eval_function.function_definition = node;
	stack_push(&stack, fn);
	return fn; // TODO: do we need to modify parser to make fn def an expression, so can call anonymous function directly after definition fn (a,b) { return a + b}(2, 2) ??
}

static struct EvalValue printBuiltin(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);

	struct EvalValue val;
	val.type = VALUE_INTEGER;
	val.value.integer = 0;

	struct AstNode **expressions = node->node.function_call.arguments->node.list_expression.list;
	for (size_t i = 0; i < vector_size(expressions); i++)
	{
		val = evalExpression(expressions[i], offset);
		switch (val.type)
		{
			// TODO: handle -1 returns..
			case VALUE_INTEGER:
				val.value.integer += printf("%d\n", val.value.integer);
				break ;
			case VALUE_FUNCTION:
				val.value.integer += put_string(val.value.eval_function.function_definition->node.function_definition.identifier->node.identifier.name);
				write(STDOUT_FILENO, "\n", 1);
				break ;
			case VALUE_NONE:
				val.value.integer += printf("none\n");
				break ;
			default:
				trace_display();
				assert(NULL);
		}
	}
	return val;
}

static struct EvalValue builtinFunctionCall(struct AstNode *node, size_t offset)
{
	assert(node);

	switch (node->node.function_call.identifier->node.builtin.type)
	{
		case TOKEN_PRINT:
			return printBuiltin(node, offset);
		default:
			break ;
	}
	runtime_error("unimplemented builtin '%s'", token_debug_value(node->node.function_call.identifier->node.builtin.type));
	return (struct EvalValue){}; // compiler?
}

static struct EvalValue evalFunctionCall(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);
	assert(node->type == AST_FUNCTION_CALL);

	size_t new_offset;

	struct FunctionCall *function_call = &(node->node.function_call);
	if (function_call->identifier->type == AST_BUILTIN)
		return builtinFunctionCall(node, offset);

	struct String fn_string = function_call->identifier->node.identifier.name;

	const struct EvalValue *function = stack.values + offset + function_call->identifier->node.identifier.offset; // scope offset + variable offset access in stack
	// TODO: undefined function error should be handled by the parser, test it
	if (function->type != VALUE_FUNCTION)
		runtime_error("trying to call uncallabed function '%s'", fn_string.str);

	new_offset = stack.len;
	stack_push(&stack, *function); // add function to function stack so it can be recursive

	struct AstNode **identifiers = function->value.eval_function.function_definition->node.function_definition.parameters;
	struct AstNode **expressions = node->node.function_call.arguments->node.list_expression.list;
	if (vector_size(identifiers) != vector_size(expressions))
		runtime_error("function '%s' expected %zu parameters, got %zu", fn_string.str, vector_size(identifiers), vector_size(expressions));
	for (size_t i = 0; i < vector_size(identifiers); i++)
	{
		assert(identifiers[i]->node.identifier.offset == i + 1);
		struct EvalValue value = evalExpression(expressions[i], offset);
		stack_push(&stack, value);
	}

	struct BlockEvalValue ret = evalBlockStatement(function->value.eval_function.function_definition->node.function_definition.block, new_offset);
	if (ret.type == BLOCK_NO_RETURN)
		ret.val.type = VALUE_NONE;

	stack_shrink(&stack, new_offset);

	return ret.val;
}

static struct EvalValue evalNone(struct AstNode *node, size_t offset)
{
	(void)node;
	(void)offset;
	return (struct EvalValue){.type=VALUE_NONE};
}

struct EvalValue(*eval_expression[AST_LIMIT])(struct AstNode *, size_t) = 
{
	[AST_NONE] = &evalNone,
	[AST_IDENTIFIER] = &evalIdentifier,
	[AST_INTEGER_LITERAL] = &evalIntegerLiteral,
	[AST_BINARY_OP] = &evalBinaryOp,
	[AST_UNARY_OP] = &evalUnaryOp,
	[AST_FUNCTION_CALL] = &evalFunctionCall,
};

// never returns NULL, should crash before
static struct EvalValue evalExpression(struct AstNode *node,  size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	struct EvalValue(*f)(struct AstNode *, size_t) = eval_expression[node->type];
	if (f == NULL) {
		trace_display();
		assert(NULL);
	}
	return f(node, offset);
}

static struct EvalValue evalLetStatement(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);
	assert(node->type == AST_LET_STATEMENT);

	struct EvalValue val = evalExpression(node->node.let_statement.value, offset);
	stack_push(&stack, val);
	return val;
}

static struct BlockEvalValue evalBlockStatement(struct AstNode *node, size_t offset)
{
	trace("%s: %s(%s, %zu)", EVAL_TRACER, __func__, ast_debug_value(node->type), offset);
	assert(node);
	assert(node->type == AST_BLOCK_STATEMENT);

	struct AstNode **statements = node->node.block_statement.statements;
	struct BlockEvalValue ret;

	ret.type = BLOCK_NO_RETURN;

	for (size_t i = 0; i < vector_size(statements); i++)
	{
		switch (statements[i]->type)
		{
			case AST_LET_STATEMENT:
				evalLetStatement(statements[i], offset);
				break ;
			case AST_IF_STATEMENT:
				ret = evalIfStatement(statements[i], offset);
				break ;
			case AST_WHILE_STATEMENT:
				ret = evalWhileStatement(statements[i], offset);
				break ;
			case AST_FUNCTION_DEFINITION:
				evalFunctionDefinition(statements[i], offset);
				break;
			case AST_RETURN_STATEMENT:
				ret.type = BLOCK_RETURN;
				ret.val = evalExpression(statements[i]->node.return_statement.expr, offset);
				break;
			default:
				evalExpression(statements[i], offset);
		}
		if (ret.type == BLOCK_RETURN)
			return ret;
	}
	return ret;
}

void evaluator_eval(struct Program program)
{
	trace("%s: %s", EVAL_TRACER, __func__);
	stack_init(&stack);
	for (size_t i = 0; i < vector_size(program.statements); i++)
	{
		switch(program.statements[i]->type)
		{
			case AST_LET_STATEMENT:
				evalLetStatement(program.statements[i], 0);
				continue ;
			case AST_WHILE_STATEMENT:
				evalWhileStatement(program.statements[i], 0);
				continue ;
			case AST_IF_STATEMENT:
				evalIfStatement(program.statements[i], 0);
				continue ;
			case AST_FUNCTION_DEFINITION:
				evalFunctionDefinition(program.statements[i], 0);
				continue ;
			case AST_RETURN_STATEMENT:
				runtime_error("unexpected '%s' in global scope", ast_debug_value(AST_RETURN_STATEMENT));
			default:
				evalExpression(program.statements[i], 0);
		}
	}
	stack_destroy(&stack);
	trace_display();
}

