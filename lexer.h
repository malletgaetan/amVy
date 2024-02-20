#ifndef LEXER_H
# define LEXER_H

# include "types.h"

struct Lexer {
	char *input;
	size_t index;
	size_t next_index;
};

bool lexer_init(struct Lexer *lexer, char *filepath);
void lexer_destroy(struct Lexer *lexer);
struct Program lexer_next_token(struct Lexer *lexer);

#endif
