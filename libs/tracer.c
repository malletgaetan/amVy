#include "libs/vector.h"
#include "libs/tracer.h"

char **content = NULL;

void	_trace(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	size_t needed = vsnprintf((char*)NULL, 0, fmt, args) + 2;
	va_end(args);
    char  *buffer = malloc(needed);
	va_start(args, fmt);
    vsprintf(buffer, fmt, args);
	va_end(args);
	buffer[needed - 1] = '\0';
	buffer[needed - 2] = '\n';
	vector_add(content, buffer);
}

void _trace_display(void)
{
	for (size_t i = 0; i < vector_size(content); i++)
		printf("%s", content[i]);
}

void _trace_clear(void)
{
	for (size_t i = 0; i < vector_size(content); i++)
		free(content[i]);
	vector_destroy(content);
	content = NULL;
}

