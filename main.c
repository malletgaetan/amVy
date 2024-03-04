#include <stdio.h>
#include <string.h>
#include "evaluator/evaluator.h"
#include "libs/vector.h"
#include "libs/tracer.h"
#include "parser/parser.h"
#include "ast/ast.h"

struct {
	int ast;
	const char *filepath;
} args;

static char *get_file_content(const char *filepath)
{
	char *content;
	FILE *file;
	long fsize;

	if ((file = fopen(filepath, "rb")) == NULL)
		return (NULL);
	fseek(file, 0, SEEK_END);
	if ((fsize = ftell(file)) == -1)
		goto fail_close;
	fseek(file, 0, SEEK_SET);
	content = malloc(sizeof(char) * (fsize + 1));
	if (content == NULL)
		goto fail_close;
	fread(content, fsize, 1, file); // TODO: error check
	fclose(file);
	content[(size_t)fsize] = '\0';
	return content;
fail_close:
	fclose(file);
	return NULL;
}

static void print_helper(const char *filepath)
{
	printf("usage: ./%s <amVy file>\n", filepath);
	printf("options:\n");
	printf("--ast: print ast\n");
	exit(0);
}

static void parse_arguments(int ac, char **av)
{
	if (ac == 1)
		print_helper(av[0]);
	for (int i = 0; i < ac; i++) {
		if (av[i][0] == '-' && av[i][1] == '-') {
			if (strcmp(av[i], "ast"))
				args.ast = 1;
		} else {
			args.filepath = av[i];
		}
	}
}

int main(int ac, char **av)
{
	char *file_content;
	struct Parser parser;
	struct Program program;

	parse_arguments(ac, av);
	file_content = get_file_content(args.filepath);
	if (file_content == NULL)
	{
		printf("failed to open %s\n", args.filepath);
		return (1);
	}
	parser_init(&parser, file_content);
	program = parser_parse(&parser);
	if (args.ast) {
		for (unsigned int i = 0; i < vector_size(program.statements); i++)
		{
			print_node(program.statements[i], 0);
		}
		goto quit;
	}
	evaluator_eval(program);
	quit:
	parser_destroy(&parser);
	return (0);
}

