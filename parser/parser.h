#ifndef PARSER_H
# define PARSER_H

# include "libs/types.h"
# include "ast/ast.h"
# include "lexer/lexer.h"
# include "token/token.h"

struct Parser {
	struct Lexer lexer;
	struct Token token;
	struct Token next_token;
};

void parser_init(struct Parser *parser, char *file_content);
void parser_destroy(struct Parser *parser);
struct Program parser_parse(struct Parser *parser);

#endif
