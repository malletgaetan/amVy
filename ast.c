#include <stdio.h>
#include "string.h"
#include "ast.h"

#define DEBUG_INDENT 5

char *binary_op_debug[] = 
{
	[BINARY_ADD] = "+",
	[BINARY_MINUS] = "-",
	[BINARY_MULTIPLY] = "*",
	[BINARY_DIVIDE] = "/",
};

char *unary_op_debug[] =
{
	[UNARY_MINUS] = "-",
	[UNARY_INVERSE] = "!",
};

char *ast_debug[] = 
{
	[AST_LET_STATEMENT] = "AST_LET_STATEMENT",
	[AST_IDENTIFIER] = "AST_IDENTIFIER",
	[AST_BINARY_OP] = "AST_BINARY_OP",
	[AST_UNARY_OP] = "AST_UNARY_OP",
	[AST_INTEGER_LITERAL] = "AST_INTEGER_LITERAL",
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

void print_node(struct Node *node, int i)
{
	switch (node->type)
	{
		case AST_LET_STATEMENT:
			printf("%*s[AST_LET_STATEMENT]\n",i * DEBUG_INDENT," ");
			printf("%*sidentifier:\n", ++i * DEBUG_INDENT," ");
			print_node(node->node.let_statement.identifier, i);
			printf("%*svalue:\n", i * DEBUG_INDENT," ");
			print_node(node->node.let_statement.value, i);
			break;
		case AST_BINARY_OP:
			printf("%*s[AST_BINARY_OP]\n", i * DEBUG_INDENT," ");
			printf("%*sop: %s\n", ++i * DEBUG_INDENT," ", binary_op_debug[node->node.binary_op.op]);
			printf("%*sleft:\n", i * DEBUG_INDENT," ");
			print_node(node->node.binary_op.left, i);
			printf("%*sright:\n", i * DEBUG_INDENT," ");
			print_node(node->node.binary_op.right, i);
			break;
		case AST_UNARY_OP:
			printf("%*s[AST_UNARY_OP]\n", i * DEBUG_INDENT, " ");
			printf("%*sop: %s\n", ++i * DEBUG_INDENT, " ", unary_op_debug[node->node.unary_op.op]);
			printf("%*sleft:\n", i * DEBUG_INDENT, " ");
			print_node(node->node.unary_op.value, i);
			break;
		case AST_IDENTIFIER:
			write_space(i * DEBUG_INDENT);
			printf("[AST_IDENTIFIER]\n");
			write_space(++i * DEBUG_INDENT);
			write(STDOUT_FILENO, "name: ", 6);
			put_string(node->node.identifier.name);
			write(STDOUT_FILENO, "\n", 1);
			break;
		case AST_INTEGER_LITERAL:
			printf("%*s[AST_INTEGER_LITERAL]\n", i * DEBUG_INDENT, " ");
			printf("%*svalue = %d\n", ++i * DEBUG_INDENT, " ", node->node.integer_literal.value);
			break;
		default:
			break;
	}
	return ;
}
