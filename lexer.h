#ifndef LEXER_H
# define LEXER_H

# include "types.h"

struct Lexer {
	char *input;
	size_t index;
	size_t next_index;
};

bool lexer_init(struct Lexer *lexer, char *filepath);
struct Token lexer_next_token(struct Lexer *lexer);

#endif
