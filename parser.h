#ifndef PARSER_H
# define PARSER_H

# include "types.h"
# include "ast.h"
# include "lexer.h"
# include "token.h"

struct Parser {
	struct Lexer lexer;
	struct Token token;
	struct Token next_token;
};

bool parser_init(struct Parser *parser, char *filepath);
void parser_destroy(struct Parser *parser);
struct Program parser_parse(struct Parser *parser);

#endif
