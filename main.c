#include <stdio.h>
#include "libs/vector.h"
#include "parser/parser.h"
#include "ast/ast.h"

char *get_file_content(const char *filepath)
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

int main(int ac, char **av)
{
	char *file_content;
	struct Parser parser;
	struct Program program;

	if (ac < 2)
	{
		printf("usage: ./%s <amVy file>\n", av[0]);
		return (1);
	}

	file_content = get_file_content(av[1]);
	if (file_content == NULL)
	{
		printf("failed to open %s\n", av[1]);
		return (1);
	}

	parser_init(&parser, file_content);

	program = parser_parse(&parser);
	for (unsigned int i = 0; i < vector_size(program.statements); i++)
	{
		print_node(program.statements[i], 0);
	}
	parser_destroy(&parser);

	return (0);
}

