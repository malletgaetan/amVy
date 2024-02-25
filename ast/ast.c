#include <stdio.h>
#include "libs/vector.h"
#include "libs/string.h"
#include "ast/ast.h"

#define DEBUG_INDENT 5

char *op_debug[] = 
{
	[BINARY_ADD] = "BINARY +",
	[BINARY_MINUS] = "BINARY -",
	[BINARY_MULTIPLY] = "BINARY *",
	[BINARY_DIVIDE] = "BINARY /",
	[BINARY_MODULO] = "BINARY %",
	[UNARY_MINUS] = "UNARY -",
	[BINARY_EQUAL] = "==",
	[BINARY_NOT_EQUAL] = "!=",
	[BINARY_LT] = "<",
	[BINARY_GT] = ">",
	[BINARY_ASSIGN] = "=",
};

char *ast_debug[] = 
{
	[AST_LET_STATEMENT] = "AST_LET_STATEMENT",
	[AST_IDENTIFIER] = "AST_IDENTIFIER",
	[AST_BINARY_OP] = "AST_BINARY_OP",
	[AST_UNARY_OP] = "AST_UNARY_OP",
	[AST_INTEGER_LITERAL] = "AST_INTEGER_LITERAL",
	[AST_RETURN_STATEMENT] = "AST_RETURN_STATEMENT",
	[AST_ARRAY_ACCESS] = "AST_ARRAY_ACCESS",
	[AST_FUNCTION_CALL] = "AST_FUNCTION_CALL",
	[AST_LIST_EXPRESSION] = "AST_LIST_EXPRESSION",
	[AST_IF_STATEMENT] = "AST_IF_STATEMENT",
	[AST_BLOCK_STATEMENT] = "AST_BLOCK_STATEMENT",
	[AST_FUNCTION_DEFINITION] = "AST_FUNCTION_DEFINITION",
};

const char *ast_debug_value(enum NodeType type)
{
	return ast_debug[type];
}

void write_space(int i)
{
	while (i--)
		write(STDOUT_FILENO, " ", 1);
}


// TODO: make this function look good
void print_node(struct AstNode *node, int i)
{
	switch (node->type)
	{
		case AST_FUNCTION_DEFINITION:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_FUNCTION_DEFINITION]);
			printf("%*sidentifier:\n", ++i * DEBUG_INDENT, " ");
			print_node(node->node.function_definition.identifier, i);
			printf("%*sparameters:\n", i * DEBUG_INDENT, " ");
			for (size_t j = 0; j < vector_size(node->node.function_definition.parameters); j++)
			{
				printf("%*s ---- %zu ----\n", i * DEBUG_INDENT, " ", j);
				print_node(node->node.function_definition.parameters[j], i);
			}
			printf("%*sblock:\n", i * DEBUG_INDENT, " ");
			print_node(node->node.function_definition.block, i);
			break;
		case AST_IF_STATEMENT:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_IF_STATEMENT]);
			printf("%*scondition:\n", ++i * DEBUG_INDENT," ");
			print_node(node->node.if_statement.cond, i);
			printf("%*sblock:\n", i * DEBUG_INDENT," ");
			print_node(node->node.if_statement.block, i);
			if (node->node.if_statement.else_block)
			{
				printf("%*selse_block:\n", i * DEBUG_INDENT," ");
				print_node(node->node.if_statement.else_block, i);
			}
			break;
		case AST_BLOCK_STATEMENT:
			printf("%*s[%s]\n", i++ * DEBUG_INDENT, " ", ast_debug[AST_BLOCK_STATEMENT]);
			for (size_t j = 0; j < vector_size(node->node.block_statement.statements); j++)
			{
				printf("%*s ---- %zu ----\n", i * DEBUG_INDENT, " ", j);
				print_node(node->node.block_statement.statements[j], i);
			}
			break;
		case AST_LET_STATEMENT:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_LET_STATEMENT]);
			printf("%*sidentifier:\n", ++i * DEBUG_INDENT," ");
			print_node(node->node.let_statement.identifier, i);
			printf("%*svalue:\n", i * DEBUG_INDENT," ");
			print_node(node->node.let_statement.value, i);
			break;
		case AST_LIST_EXPRESSION:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_LIST_EXPRESSION]);
			printf("%*ssize: %zu\n", ++i * DEBUG_INDENT, " ", vector_size(node->node.list_expression.list));
			printf("%*selements:\n", i * DEBUG_INDENT," ");
			for (unsigned int j = 0; j < vector_size(node->node.list_expression.list); j++)
			{
				printf("%*s ---- %u ----\n", i * DEBUG_INDENT, " ", j);
				print_node(node->node.list_expression.list[j], i);
			}
			break;
		case AST_FUNCTION_CALL:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_FUNCTION_CALL]);
			printf("%*sidentifier:\n", ++i * DEBUG_INDENT," ");
			print_node(node->node.function_call.identifier, i);
			printf("%*sindex:\n", i * DEBUG_INDENT," ");
			print_node(node->node.function_call.arguments, i);
			break;
		case AST_ARRAY_ACCESS:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_ARRAY_ACCESS]);
			printf("%*sidentifier:\n", ++i * DEBUG_INDENT," ");
			print_node(node->node.array_access.identifier, i);
			printf("%*sindex:\n", i * DEBUG_INDENT," ");
			print_node(node->node.array_access.index, i);
			break;
		case AST_BINARY_OP:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_BINARY_OP]);
			printf("%*sop: %s\n", ++i * DEBUG_INDENT, " ", op_debug[node->node.binary_op.op]);
			printf("%*sleft:\n", i * DEBUG_INDENT," ");
			print_node(node->node.binary_op.left, i);
			printf("%*sright:\n", i * DEBUG_INDENT," ");
			print_node(node->node.binary_op.right, i);
			break;
		case AST_UNARY_OP:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_UNARY_OP]);
			printf("%*sop: %s\n", ++i * DEBUG_INDENT, " ", op_debug[node->node.unary_op.op]);
			printf("%*sleft:\n", i * DEBUG_INDENT, " ");
			print_node(node->node.unary_op.value, i);
			break;
		case AST_IDENTIFIER:
			write_space(i * DEBUG_INDENT);
			printf("[%s]\n", ast_debug[AST_INTEGER_LITERAL]);
			write_space(++i * DEBUG_INDENT);
			write(STDOUT_FILENO, "name: ", 6);
			put_string(node->node.identifier.name);
			write(STDOUT_FILENO, "\n", 1);
			break;
		case AST_INTEGER_LITERAL:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_INTEGER_LITERAL]);
			printf("%*svalue = %d\n", ++i * DEBUG_INDENT, " ", node->node.integer_literal.value);
			break;
		case AST_RETURN_STATEMENT:
			printf("%*s[%s]\n", i * DEBUG_INDENT, " ", ast_debug[AST_RETURN_STATEMENT]);
			printf("%*sreturn expr:\n", i * DEBUG_INDENT," ");
			print_node(node->node.return_statement.expr, i);
			break;
		default:
			break;
	}
	return ;
}

