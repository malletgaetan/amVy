#include <string.h>
#include <assert.h>
#include "parser/context.h"

static int user_compare(const void *a, const void *b, void *udata) {
	(void)udata;
    const struct AstNode *sa = a;
    const struct AstNode *sb = b;
	if (sa->node.identifier.name.size == sb->node.identifier.name.size)
		return strncmp(sa->node.identifier.name.str, sb->node.identifier.name.str, sa->node.identifier.name.size);
	return (sa->node.identifier.name.size < sb->node.identifier.name.size ? -1 : 1);
}

static uint64_t user_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct AstNode *val = item;
    return hashmap_sip(val->node.identifier.name.str, val->node.identifier.name.size, seed0, seed1);
}

void context_init(struct Context *ctx)
{
	ctx->offset = 0;
	ctx->hashmap = hashmap_new(sizeof(struct AstNode), 0, 0, 0, user_hash, user_compare, NULL, NULL);
	assert(ctx->hashmap);
}

void context_destroy(struct Context *ctx)
{
	hashmap_free(ctx->hashmap);
}

struct AstNode *context_get_addr(struct Context *ctx, const struct String str)
{
	struct AstNode key;

	key.node.identifier.name = str;

	return (struct AstNode *)hashmap_get(ctx->hashmap, &key);
}

struct AstNode *context_set_addr(struct Context *ctx, struct AstNode *node)
{
	node->node.identifier.offset = ctx->offset++;
	hashmap_set(ctx->hashmap, node);
	return (struct AstNode *)hashmap_get(ctx->hashmap, node);
}
