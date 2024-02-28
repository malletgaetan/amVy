#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <libs/vector.h>
#include "libs/string.h"
#include "evaluator/evaluator.h"

char *eval_debug[] = {
	[VALUE_INTEGER] = "INTEGER",
	[VALUE_FUNCTION] = "FUNCTION",
};


static struct BlockEvalValue evalBlockStatement(struct AstNode *node, struct hashmap *context);
static struct EvalValue evalExpression(struct AstNode *node,  struct hashmap *context);

int user_compare(const void *a, const void *b, void *udata) {
	(void)udata;
    const struct EvalValueMap *sa = a;
    const struct EvalValueMap *sb = b;
	if (sa->identifier.size == sb->identifier.size)
		return strncmp(sa->identifier.str, sb->identifier.str, sa->identifier.size);
	return (sa->identifier.size < sb->identifier.size ? -1 : 1);
}

bool user_iter(const void *item, void *udata) {
	(void)item;
	(void)udata;
    /* const struct user *user = item; */
    /* printf("%s (age=%d)\n", user->name, user->age); */
    return true;
}

uint64_t user_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct EvalValueMap *val = item;
    return hashmap_sip(val->identifier.str, val->identifier.size, seed0, seed1);
}

static void binaryOpTypeCheck(struct EvalValue left, struct EvalValue right)
{
	if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER)
		return ;
	printf("runtime_error: invalid arithmetic operation between '%s' and '%s' values\n", eval_debug[left.type], eval_debug[left.type]);
	exit(1);
}

static void expectIdentifier(struct AstNode *node)
{
	assert(node);
	if (node->type == AST_IDENTIFIER)
		return ;
	printf("runtime_error: expected identifier\n");
	exit(1);
}

static struct EvalValue evalBinaryOp(struct AstNode *node, struct hashmap *context)
{
	assert(node);
	assert(context);

	if (node->node.binary_op.op == BINARY_ASSIGN)
	{
		expectIdentifier(node->node.binary_op.left);
		struct EvalValue right = evalExpression(node->node.binary_op.right, context);
		struct EvalValueMap map_val;
		map_val.identifier = node->node.binary_op.left->node.identifier.name;
		map_val.val = right;
		hashmap_set(context, &map_val);
		return right;
	}

	struct EvalValue left = evalExpression(node->node.binary_op.left, context);
	struct EvalValue right = evalExpression(node->node.binary_op.right, context);

	binaryOpTypeCheck(left, right);

	switch (node->node.binary_op.op)
	{
		case BINARY_ADD:
			left.value.integer += right.value.integer;
			break;
		case BINARY_MINUS:
			left.value.integer -= right.value.integer;
			break;
		case BINARY_MULTIPLY:
			left.value.integer *= right.value.integer;
			break;
		case BINARY_DIVIDE:
			left.value.integer /= right.value.integer;
			break;
		case BINARY_MODULO:
			left.value.integer %= right.value.integer;
			break;
		case BINARY_EQUAL:
			left.value.integer = left.value.integer == right.value.integer;
			break;
		case BINARY_NOT_EQUAL:
			left.value.integer = left.value.integer != right.value.integer;
			break;
		case BINARY_LT:
			left.value.integer = left.value.integer < right.value.integer;
			break;
		case BINARY_GT:
			left.value.integer = left.value.integer > right.value.integer;
			break;
		default:
			printf("runtime_error: bad operator %d\n", node->node.binary_op.op);
			exit(1);
	}
	return left;
}

static struct EvalValue evalIdentifier(struct AstNode *node, struct hashmap *context)
{
	assert(node);
	assert(context);
	assert(node->type == AST_IDENTIFIER);

	struct EvalValueMap key;
	struct EvalValue ret;

	key.identifier = node->node.identifier.name;
	const struct EvalValueMap *el = hashmap_get(context, &key);
	if (el == NULL)
	{
		printf("runtime_error: no value error\n");
		exit(1);
	}
	ret.type = VALUE_INTEGER;
	ret.value.integer = el->val.value.integer;
	return ret;
}

static struct EvalValue evalUnaryOp(struct AstNode *node, struct hashmap *context)
{
	assert(node);

	struct EvalValue val = evalExpression(node->node.unary_op.value, context);

	if (val.type != VALUE_INTEGER)
	{
		printf("runtime_error: unary operator expected integer\n");
		exit(1);
	}
	val.value.integer = -val.value.integer;
	return val;
}

static struct EvalValue evalIntegerLiteral(struct AstNode *node)
{
	assert(node);

	struct EvalValue val;

	val.type = VALUE_INTEGER;
	val.value.integer = node->node.integer_literal.value;
	return val;
}

static struct BlockEvalValue evalIfStatement(struct AstNode *node, struct hashmap *context)
{
	assert(node);
	assert(context);
	assert(node->type == AST_IF_STATEMENT);

	struct EvalValue cond = evalExpression(node->node.if_statement.cond, context);
	struct BlockEvalValue ret;
	ret.type = BLOCK_NO_RETURN;

	if (cond.type != VALUE_INTEGER)
	{
		printf("(⌐ ͡■ ͜   ͡■) dsl tu rentres pas monsieur\n");
		exit(1);
	}
	if (!cond.value.integer && node->node.if_statement.else_block)
		return evalBlockStatement(node->node.if_statement.else_block, context);
	if (cond.value.integer)
		return evalBlockStatement(node->node.if_statement.block, context);
	return ret;
}

static struct EvalValue evalFunctionDefinition(struct AstNode *node, struct hashmap *context)
{
	assert(node);
	assert(context);
	assert(node->type == AST_FUNCTION_DEFINITION);

	struct EvalValueMap map_val;

	map_val.identifier = node->node.function_definition.identifier->node.identifier.name;
	map_val.val.type = VALUE_FUNCTION;
	map_val.val.value.eval_function.function_definition = node;
	hashmap_set(context, &map_val);
	return map_val.val; // TODO: need to modify parser to make fn def an expression, so can call anonymous function directly after definition fn (a,b) { return a + b}(2, 2)
}

static struct EvalValue printBuiltin(struct AstNode *node, struct hashmap *context)
{
	assert(node);
	assert(context);
	assert(node->type == AST_FUNCTION_CALL);

	struct EvalValue val;
	val.type = VALUE_INTEGER;
	val.value.integer = 0;

	struct AstNode **expressions = node->node.function_call.arguments->node.list_expression.list;
	for (size_t i = 0; i < vector_size(expressions); i++)
	{
		val = evalExpression(expressions[i], context);
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
				assert(NULL);
		}
	}
	return val;
}

static struct EvalValue evalFunctionCall(struct AstNode *node, struct hashmap *context)
{
	assert(node);
	assert(context);
	assert(node->type == AST_FUNCTION_CALL);


	struct String fn_string = node->node.function_call.identifier->node.identifier.name;
	if (fn_string.size == 5 && strncmp(fn_string.str, "print", 5) == 0)
		return printBuiltin(node, context);
	struct hashmap *new_context = hashmap_new(sizeof(struct EvalValueMap), 0, 0, 0, user_hash, user_compare, NULL, NULL);
	assert(new_context);
	struct EvalValueMap req;
	req.identifier = fn_string;
	const struct EvalValueMap *function_def = hashmap_get(context, &req);
	if (function_def == NULL)
	{
		printf("where function?!\n");
		exit(1);
	}
	assert(function_def->val.type == VALUE_FUNCTION);
	hashmap_set(new_context, function_def);
	struct AstNode **identifiers = function_def->val.value.eval_function.function_definition->node.function_definition.parameters;
	struct AstNode **expressions = node->node.function_call.arguments->node.list_expression.list;
	if (vector_size(identifiers) != vector_size(expressions))
	{
		printf("needed more or less parameters, gl with that one\n");
		exit(1); // TODO: write a clean exit
	}
	for (size_t i = 0; i < vector_size(identifiers); i++)
	{
		req.identifier = identifiers[i]->node.identifier.name;
		req.val = evalExpression(expressions[i], context);
		hashmap_set(new_context, &req);
	}
	struct BlockEvalValue ret = evalBlockStatement(function_def->val.value.eval_function.function_definition->node.function_definition.block, new_context);
	if (ret.type == BLOCK_NO_RETURN)
		ret.val.type = VALUE_NONE;
	hashmap_free(new_context);
	return ret.val;
}

// never returns NULL, should crash before
static struct EvalValue evalExpression(struct AstNode *node,  struct hashmap *context)
{
	switch (node->type)
	{
		case AST_IDENTIFIER:
			return evalIdentifier(node, context);
		case AST_BINARY_OP:
			return evalBinaryOp(node, context);
		case AST_UNARY_OP:
			return evalUnaryOp(node, context);
		case AST_INTEGER_LITERAL:
			return evalIntegerLiteral(node);
		case AST_ARRAY_ACCESS:
			assert(NULL); // TODO: support for arrays
		case AST_FUNCTION_CALL:
			return evalFunctionCall(node, context);
		case AST_LIST_EXPRESSION:
			assert(NULL); // TODO
		default:
			printf("calling evalExpression giving no expression :s\n");
			exit(1);
	}
}

static struct EvalValue evalLetStatement(struct AstNode *node, struct hashmap *context)
{
	assert(node);
	assert(context);
	assert(node->type == AST_LET_STATEMENT);

	struct EvalValueMap req;
	req.identifier = node->node.let_statement.identifier->node.identifier.name;
	req.val = evalExpression(node->node.let_statement.value, context);
	hashmap_set(context, &req);
	return req.val;
}

static struct BlockEvalValue evalBlockStatement(struct AstNode *node, struct hashmap *context)
{
	assert(node);
	assert(context);
	assert(node->type == AST_BLOCK_STATEMENT);

	struct AstNode **statements = node->node.block_statement.statements;
	struct BlockEvalValue ret;
	ret.type = BLOCK_NO_RETURN;

	for (size_t i = 0; i < vector_size(statements); i++)
	{
		switch (statements[i]->type)
		{
			case AST_LET_STATEMENT:
				evalLetStatement(statements[i], context);
				break ;
			case AST_IF_STATEMENT:
				ret = evalIfStatement(statements[i], context);
				break ;
			case AST_FUNCTION_DEFINITION:
				evalFunctionDefinition(statements[i], context);
				break;
			case AST_RETURN_STATEMENT:
				ret.type = BLOCK_RETURN;
				ret.val = evalExpression(statements[i]->node.return_statement.expr, context);
				break;
			default:
				evalExpression(node, context);
		}
		if (ret.type == BLOCK_RETURN)
			return ret;
	}
	return ret;
}

void evaluator_eval(struct Program program)
{
	struct hashmap *context = hashmap_new(sizeof(struct EvalValueMap), 0, 0, 0, user_hash, user_compare, NULL, NULL);
	struct EvalValue val;
	for (size_t i = 0; i < vector_size(program.statements); i++)
	{
		switch(program.statements[i]->type)
		{
			case AST_LET_STATEMENT:
				val = evalLetStatement(program.statements[i], context);
				continue ;
			case AST_IF_STATEMENT:
				evalLetStatement(program.statements[i], context);
				continue ;
			case AST_FUNCTION_DEFINITION:
				evalFunctionDefinition(program.statements[i], context);
				continue ;
			case AST_RETURN_STATEMENT:
				printf("return in global scope? nice\n");
				exit(1);
			default:
				evalExpression(program.statements[i], context);
		}
	}
	hashmap_free(context);
}

