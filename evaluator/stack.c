#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "evaluator/evaluator.h"
#include "evaluator/stack.h"

// we'll use heap for stack for simplicity, compiler will not do that

void runtime_error(const char *fmt, ...);

void stack_init(struct Stack *s)
{
	s->values = malloc(100000 * sizeof(struct EvalValue));
	s->limit = 100000;
	s->len = 0;
}

void stack_destroy(struct Stack *s)
{
	free(s->values);
	s->values = NULL;
	s->limit = 0;
	s->len = 0;
}

size_t stack_push(struct Stack *s, struct EvalValue val)
{
	if (s->len == s->limit)
	{
		s->limit *= 10;
		s->values = realloc(s->values, s->limit * sizeof(struct EvalValue));
		assert(s->values);
	}
	s->values[s->len] = val;
	return (s->len++);
}

void stack_shrink(struct Stack *s, size_t new_len)
{
	s->len = new_len;
}
