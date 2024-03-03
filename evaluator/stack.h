#ifndef STACK_H

struct Stack {
	struct EvalValue *values;
	size_t len;
	size_t limit;
};

void stack_init(struct Stack *s);
void stack_destroy(struct Stack *s);
size_t stack_push(struct Stack *s, struct EvalValue val);
void stack_shrink(struct Stack *s, size_t new_len);

#endif
