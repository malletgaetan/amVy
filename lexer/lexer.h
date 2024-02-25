#ifndef LEXER_H
# define LEXER_H

# include <stddef.h>
# include "libs/types.h"

struct Lexer {
	char *input;
	size_t index;
	size_t next_index;
};

void lexer_init(struct Lexer *lexer, char *file_content);
void lexer_destroy(struct Lexer *lexer);
struct Token lexer_next_token(struct Lexer *lexer);

#endif
