#ifndef CONTEXT_H

# include "libs/hashmap.h"
# include "ast/ast.h"

struct Context {
	struct hashmap *hashmap;
	size_t offset;
};

void context_init(struct Context *ctx);
void context_destroy(struct Context *ctx); // don't destroy!!
struct AstNode *context_get_addr(struct Context *ctx, const struct String str);
struct AstNode *context_set_addr(struct Context *ctx, struct AstNode *node);

#endif
